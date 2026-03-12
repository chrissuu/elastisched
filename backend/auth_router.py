from __future__ import annotations

import uuid
from collections import defaultdict
from datetime import timedelta

from fastapi import APIRouter, Depends, HTTPException, Request, Response, status
from sqlalchemy import select
from sqlalchemy.ext.asyncio import AsyncSession

from backend.auth import (
    AuthenticatedUser,
    coerce_utc,
    apply_session_cookie,
    clear_session_cookie,
    hash_password,
    issue_session_tokens,
    normalize_email,
    require_authenticated_user,
    revoke_session,
    utcnow,
    validate_email,
    validate_password_strength,
    verify_password,
)
from backend.auth_db import get_auth_session
from backend.auth_models import UserAccountModel
from backend.auth_schemas import (
    LoginRequest,
    RegisterRequest,
    SessionRead,
    UserProfileRead,
    UserProfileUpdate,
)


auth_router = APIRouter(prefix="/auth", tags=["auth"])
FAILED_LOGIN_WINDOW = timedelta(minutes=10)
FAILED_LOGIN_LOCKOUT = timedelta(minutes=15)
FAILED_LOGIN_LIMIT = 6
_FAILED_LOGINS: dict[str, list] = defaultdict(list)


def _user_profile_from_model(model: UserAccountModel) -> UserProfileRead:
    return UserProfileRead(
        id=model.id,
        email=model.email,
        display_name=model.display_name,
        created_at=coerce_utc(model.created_at) or utcnow(),
    )


def _session_read(
    *,
    user: UserAccountModel | None = None,
    auth: AuthenticatedUser | None = None,
    csrf_token: str,
    expires_at,
) -> SessionRead:
    if user is not None:
        profile = _user_profile_from_model(user)
    elif auth is not None:
        profile = UserProfileRead(
            id=auth.user_id,
            email=auth.email,
            display_name=auth.display_name,
            created_at=auth.created_at,
        )
    else:
        raise ValueError("Either user or auth must be provided.")
    return SessionRead(user=profile, csrf_token=csrf_token, expires_at=expires_at)


def _login_attempt_key(request: Request, email: str) -> str:
    forwarded_for = str(request.headers.get("x-forwarded-for") or "").split(",")[0].strip()
    source_ip = forwarded_for or (request.client.host if request.client else "unknown")
    return f"{source_ip}|{email}"


def _assert_login_allowed(request: Request, email: str) -> None:
    key = _login_attempt_key(request, email)
    now = utcnow()
    attempts = [
        timestamp
        for timestamp in _FAILED_LOGINS.get(key, [])
        if now - timestamp <= FAILED_LOGIN_LOCKOUT
    ]
    _FAILED_LOGINS[key] = attempts
    recent_failures = [timestamp for timestamp in attempts if now - timestamp <= FAILED_LOGIN_WINDOW]
    if len(recent_failures) >= FAILED_LOGIN_LIMIT:
        raise HTTPException(
            status_code=status.HTTP_429_TOO_MANY_REQUESTS,
            detail="Too many login attempts. Try again in a few minutes.",
        )


def _record_login_failure(request: Request, email: str) -> None:
    key = _login_attempt_key(request, email)
    _FAILED_LOGINS[key].append(utcnow())


def _clear_login_failures(request: Request, email: str) -> None:
    key = _login_attempt_key(request, email)
    if key in _FAILED_LOGINS:
        del _FAILED_LOGINS[key]


@auth_router.post(
    "/register",
    response_model=SessionRead,
    status_code=status.HTTP_201_CREATED,
    operation_id="auth_register",
)
async def register(
    payload: RegisterRequest,
    request: Request,
    response: Response,
    session: AsyncSession = Depends(get_auth_session),
) -> SessionRead:
    email = normalize_email(payload.email)
    if not validate_email(email):
        raise HTTPException(
            status_code=status.HTTP_422_UNPROCESSABLE_ENTITY,
            detail="Please provide a valid email address.",
        )
    valid_password, password_error = validate_password_strength(payload.password)
    if not valid_password:
        raise HTTPException(
            status_code=status.HTTP_422_UNPROCESSABLE_ENTITY,
            detail=password_error,
        )
    existing = await session.execute(
        select(UserAccountModel).where(UserAccountModel.email == email)
    )
    if existing.scalar_one_or_none() is not None:
        raise HTTPException(
            status_code=status.HTTP_409_CONFLICT,
            detail="An account with this email already exists.",
        )

    user = UserAccountModel(
        id=str(uuid.uuid4()),
        email=email,
        password_hash=hash_password(payload.password),
        display_name=(payload.display_name or "").strip() or None,
        created_at=utcnow(),
    )
    raw_session_token, session_row = issue_session_tokens(user.id)
    session.add(user)
    session.add(session_row)
    await session.commit()
    await session.refresh(user)

    apply_session_cookie(response, request, raw_session_token, session_row.expires_at)
    return _session_read(
        user=user,
        csrf_token=session_row.csrf_token,
        expires_at=session_row.expires_at,
    )


@auth_router.post(
    "/login",
    response_model=SessionRead,
    operation_id="auth_login",
)
async def login(
    payload: LoginRequest,
    request: Request,
    response: Response,
    session: AsyncSession = Depends(get_auth_session),
) -> SessionRead:
    email = normalize_email(payload.email)
    _assert_login_allowed(request, email)
    result = await session.execute(
        select(UserAccountModel).where(UserAccountModel.email == email)
    )
    user = result.scalar_one_or_none()
    if user is None or not verify_password(payload.password, user.password_hash):
        _record_login_failure(request, email)
        raise HTTPException(
            status_code=status.HTTP_401_UNAUTHORIZED,
            detail="Invalid email or password.",
        )
    _clear_login_failures(request, email)

    raw_session_token, session_row = issue_session_tokens(user.id)
    session.add(session_row)
    await session.commit()
    apply_session_cookie(response, request, raw_session_token, session_row.expires_at)
    return _session_read(
        user=user,
        csrf_token=session_row.csrf_token,
        expires_at=session_row.expires_at,
    )


@auth_router.get(
    "/me",
    response_model=SessionRead,
    operation_id="auth_me",
)
async def me(
    auth: AuthenticatedUser = Depends(require_authenticated_user),
) -> SessionRead:
    return _session_read(
        auth=auth,
        csrf_token=auth.csrf_token,
        expires_at=auth.expires_at,
    )


@auth_router.patch(
    "/profile",
    response_model=UserProfileRead,
    operation_id="auth_update_profile",
)
async def update_profile(
    payload: UserProfileUpdate,
    auth: AuthenticatedUser = Depends(require_authenticated_user),
    session: AsyncSession = Depends(get_auth_session),
) -> UserProfileRead:
    result = await session.execute(
        select(UserAccountModel).where(UserAccountModel.id == auth.user_id)
    )
    user = result.scalar_one_or_none()
    if user is None:
        raise HTTPException(
            status_code=status.HTTP_404_NOT_FOUND,
            detail="User not found.",
        )
    if payload.display_name is not None:
        user.display_name = payload.display_name.strip() or None
    await session.commit()
    await session.refresh(user)
    return _user_profile_from_model(user)


@auth_router.post(
    "/logout",
    status_code=status.HTTP_204_NO_CONTENT,
    operation_id="auth_logout",
)
async def logout(
    request: Request,
    response: Response,
    auth: AuthenticatedUser = Depends(require_authenticated_user),
    session: AsyncSession = Depends(get_auth_session),
) -> Response:
    await revoke_session(session, auth.session_id)
    clear_session_cookie(response, request)
    response.status_code = status.HTTP_204_NO_CONTENT
    return response
