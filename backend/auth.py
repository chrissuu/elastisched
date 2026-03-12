from __future__ import annotations

import base64
import hashlib
import hmac
import re
import secrets
import uuid
from dataclasses import dataclass
from datetime import datetime, timedelta, timezone

from fastapi import Depends, HTTPException, Request, status
from sqlalchemy import select
from sqlalchemy.ext.asyncio import AsyncSession

from backend.auth_db import get_auth_session
from backend.auth_models import UserAccountModel, UserSessionModel
from backend.config import (
    get_auth_cookie_name,
    get_password_min_length,
    get_password_pepper,
    get_session_token_secret,
    get_session_ttl_hours,
)


SAFE_METHODS = {"GET", "HEAD", "OPTIONS"}
CSRF_HEADER = "x-csrf-token"
SESSION_IDLE_UPDATE_SECONDS = 300
EMAIL_PATTERN = re.compile(r"^[A-Za-z0-9._%+\-]+@[A-Za-z0-9.\-]+\.[A-Za-z]{2,}$")
SCRYPT_N = 2**14
SCRYPT_R = 8
SCRYPT_P = 1
SCRYPT_DKLEN = 64


@dataclass(frozen=True, slots=True)
class AuthenticatedUser:
    user_id: str
    email: str
    display_name: str | None
    created_at: datetime
    session_id: str
    csrf_token: str
    expires_at: datetime


def utcnow() -> datetime:
    return datetime.now(timezone.utc)


def coerce_utc(value: datetime | None) -> datetime | None:
    if value is None:
        return None
    if value.tzinfo is None:
        return value.replace(tzinfo=timezone.utc)
    return value.astimezone(timezone.utc)


def normalize_email(email: str) -> str:
    return str(email or "").strip().lower()


def validate_email(email: str) -> bool:
    return bool(EMAIL_PATTERN.fullmatch(email))


def validate_password_strength(password: str) -> tuple[bool, str]:
    minimum = get_password_min_length()
    if len(password) < minimum:
        return False, f"Password must be at least {minimum} characters."
    if not any(char.islower() for char in password):
        return False, "Password must include a lowercase letter."
    if not any(char.isupper() for char in password):
        return False, "Password must include an uppercase letter."
    if not any(char.isdigit() for char in password):
        return False, "Password must include a number."
    return True, ""


def _password_bytes(password: str) -> bytes:
    pepper = get_password_pepper()
    return f"{password}{pepper}".encode("utf-8")


def hash_password(password: str) -> str:
    salt = secrets.token_bytes(16)
    digest = hashlib.scrypt(
        _password_bytes(password),
        salt=salt,
        n=SCRYPT_N,
        r=SCRYPT_R,
        p=SCRYPT_P,
        dklen=SCRYPT_DKLEN,
    )
    salt_b64 = base64.urlsafe_b64encode(salt).decode("ascii")
    digest_b64 = base64.urlsafe_b64encode(digest).decode("ascii")
    return f"scrypt${SCRYPT_N}${SCRYPT_R}${SCRYPT_P}${salt_b64}${digest_b64}"


def verify_password(password: str, encoded_hash: str) -> bool:
    try:
        algorithm, n_raw, r_raw, p_raw, salt_b64, digest_b64 = encoded_hash.split("$", 5)
        if algorithm != "scrypt":
            return False
        n = int(n_raw)
        r = int(r_raw)
        p = int(p_raw)
        salt = base64.urlsafe_b64decode(salt_b64.encode("ascii"))
        expected = base64.urlsafe_b64decode(digest_b64.encode("ascii"))
    except Exception:
        return False
    derived = hashlib.scrypt(
        _password_bytes(password),
        salt=salt,
        n=n,
        r=r,
        p=p,
        dklen=len(expected),
    )
    return hmac.compare_digest(derived, expected)


def _token_secret_bytes() -> bytes:
    return get_session_token_secret().encode("utf-8")


def hash_session_token(raw_token: str) -> str:
    return hmac.new(
        _token_secret_bytes(),
        raw_token.encode("utf-8"),
        hashlib.sha256,
    ).hexdigest()


def issue_session_tokens(user_id: str) -> tuple[str, UserSessionModel]:
    now = utcnow()
    ttl = timedelta(hours=get_session_ttl_hours())
    raw_token = secrets.token_urlsafe(48)
    csrf_token = secrets.token_urlsafe(24)
    session_row = UserSessionModel(
        id=str(uuid.uuid4()),
        user_id=user_id,
        token_hash=hash_session_token(raw_token),
        csrf_token=csrf_token,
        created_at=now,
        last_seen_at=now,
        expires_at=now + ttl,
        revoked_at=None,
    )
    return raw_token, session_row


def request_is_secure(request: Request) -> bool:
    if request.url.scheme == "https":
        return True
    forwarded_proto = str(request.headers.get("x-forwarded-proto") or "").split(",")[0].strip()
    return forwarded_proto.lower() == "https"


def apply_session_cookie(response, request: Request, raw_token: str, expires_at: datetime) -> None:
    now = utcnow()
    max_age = int(max(0, (expires_at - now).total_seconds()))
    response.set_cookie(
        key=get_auth_cookie_name(),
        value=raw_token,
        max_age=max_age,
        expires=max_age,
        path="/",
        httponly=True,
        secure=request_is_secure(request),
        samesite="lax",
    )


def clear_session_cookie(response, request: Request) -> None:
    response.set_cookie(
        key=get_auth_cookie_name(),
        value="",
        max_age=0,
        expires=0,
        path="/",
        httponly=True,
        secure=request_is_secure(request),
        samesite="lax",
    )


async def revoke_session(session: AsyncSession, session_id: str) -> None:
    result = await session.execute(
        select(UserSessionModel).where(UserSessionModel.id == session_id)
    )
    row = result.scalar_one_or_none()
    if row is None:
        return
    row.revoked_at = utcnow()
    await session.commit()


async def require_authenticated_user(
    request: Request,
    session: AsyncSession = Depends(get_auth_session),
) -> AuthenticatedUser:
    raw_token = str(request.cookies.get(get_auth_cookie_name()) or "").strip()
    if not raw_token:
        raise HTTPException(
            status_code=status.HTTP_401_UNAUTHORIZED,
            detail="Authentication required",
        )

    now = utcnow()
    token_hash = hash_session_token(raw_token)
    result = await session.execute(
        select(UserSessionModel, UserAccountModel)
        .join(UserAccountModel, UserSessionModel.user_id == UserAccountModel.id)
        .where(
            UserSessionModel.token_hash == token_hash,
            UserSessionModel.revoked_at.is_(None),
            UserSessionModel.expires_at > now,
        )
    )
    row = result.first()
    if row is None:
        raise HTTPException(
            status_code=status.HTTP_401_UNAUTHORIZED,
            detail="Authentication required",
        )
    session_row, user_row = row
    if request.method.upper() not in SAFE_METHODS:
        provided_csrf = str(request.headers.get(CSRF_HEADER) or "").strip()
        if not provided_csrf:
            provided_csrf = str(request.query_params.get("csrf_token") or "").strip()
        if not provided_csrf or not hmac.compare_digest(
            provided_csrf,
            session_row.csrf_token,
        ):
            raise HTTPException(
                status_code=status.HTTP_403_FORBIDDEN,
                detail="Invalid CSRF token",
            )

    last_seen_at = coerce_utc(session_row.last_seen_at)
    if last_seen_at is None or (now - last_seen_at).total_seconds() >= SESSION_IDLE_UPDATE_SECONDS:
        session_row.last_seen_at = now
        await session.commit()

    expires_at = coerce_utc(session_row.expires_at) or now

    return AuthenticatedUser(
        user_id=user_row.id,
        email=user_row.email,
        display_name=user_row.display_name,
        created_at=coerce_utc(user_row.created_at) or now,
        session_id=session_row.id,
        csrf_token=session_row.csrf_token,
        expires_at=expires_at,
    )
