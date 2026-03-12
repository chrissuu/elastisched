from __future__ import annotations

import asyncio
import re
from pathlib import Path
from typing import AsyncGenerator

from fastapi import Depends
from sqlalchemy import text
from sqlalchemy.ext.asyncio import AsyncEngine, AsyncSession, async_sessionmaker, create_async_engine
from sqlalchemy.orm import DeclarativeBase

from backend.auth import AuthenticatedUser, require_authenticated_user
from backend.config import get_database_url, get_user_workspace_dir


DATABASE_URL = get_database_url()
_ENGINE_CACHE: dict[str, AsyncEngine] = {}
_SESSION_FACTORY_CACHE: dict[str, async_sessionmaker[AsyncSession]] = {}
_INITIALIZED_URLS: set[str] = set()
_INIT_LOCK = asyncio.Lock()
_SAFE_DB_NAME_PATTERN = re.compile(r"[^a-zA-Z0-9_-]+")


class Base(DeclarativeBase):
    pass


def _engine_for_url(database_url: str) -> AsyncEngine:
    existing = _ENGINE_CACHE.get(database_url)
    if existing is not None:
        return existing
    engine = create_async_engine(database_url, pool_pre_ping=True)
    _ENGINE_CACHE[database_url] = engine
    return engine


def _session_factory_for_url(database_url: str) -> async_sessionmaker[AsyncSession]:
    existing = _SESSION_FACTORY_CACHE.get(database_url)
    if existing is not None:
        return existing
    factory = async_sessionmaker(_engine_for_url(database_url), expire_on_commit=False)
    _SESSION_FACTORY_CACHE[database_url] = factory
    return factory


def _safe_user_database_name(user_id: str) -> str:
    cleaned = _SAFE_DB_NAME_PATTERN.sub("_", str(user_id or "").strip())
    return cleaned or "default"


def _workspace_database_url(user_id: str) -> str:
    workspace_dir = Path(get_user_workspace_dir()).expanduser().resolve()
    workspace_dir.mkdir(parents=True, exist_ok=True)
    filename = f"{_safe_user_database_name(user_id)}.db"
    path = (workspace_dir / filename).resolve()
    return f"sqlite+aiosqlite:///{path}"


async def get_session(
    current_user: AuthenticatedUser = Depends(require_authenticated_user),
) -> AsyncGenerator[AsyncSession, None]:
    database_url = _workspace_database_url(current_user.user_id)
    await _init_db_url(database_url)
    session_factory = _session_factory_for_url(database_url)
    async with session_factory() as session:
        yield session


async def get_system_session() -> AsyncGenerator[AsyncSession, None]:
    await _init_db_url(DATABASE_URL)
    session_factory = _session_factory_for_url(DATABASE_URL)
    async with session_factory() as session:
        yield session


async def init_db() -> None:
    await _init_db_url(DATABASE_URL)


async def init_user_db(user_id: str) -> None:
    await _init_db_url(_workspace_database_url(user_id))


async def _init_db_url(database_url: str) -> None:
    if database_url in _INITIALIZED_URLS:
        return
    async with _INIT_LOCK:
        if database_url in _INITIALIZED_URLS:
            return
        # Ensure model metadata is registered on the current Base.
        import backend.models as models  # noqa: F401

        metadata = Base.metadata
        if not metadata.tables:
            metadata = models.BlobModel.metadata
        engine = _engine_for_url(database_url)
        async with engine.begin() as conn:
            await conn.run_sync(metadata.create_all)
            if conn.dialect.name == "sqlite":
                await _ensure_sqlite_blob_columns(conn)
                await _ensure_sqlite_scheduled_occurrence_columns(conn)
                await _ensure_sqlite_recurrence_columns(conn)
        _INITIALIZED_URLS.add(database_url)


async def _ensure_sqlite_blob_columns(conn) -> None:
    result = await conn.execute(text("PRAGMA table_info(blobs)"))
    columns = {row[1] for row in result.fetchall()}
    missing = []
    if "location" not in columns:
        missing.append(("location", "VARCHAR(500)"))
    if "realized_start" not in columns:
        missing.append(("realized_start", "DATETIME"))
    if "realized_end" not in columns:
        missing.append(("realized_end", "DATETIME"))
    for name, col_type in missing:
        await conn.execute(text(f"ALTER TABLE blobs ADD COLUMN {name} {col_type}"))


async def _ensure_sqlite_scheduled_occurrence_columns(conn) -> None:
    result = await conn.execute(text("PRAGMA table_info(scheduled_occurrences)"))
    columns = {row[1] for row in result.fetchall()}
    missing = []
    if "segment_index" not in columns:
        missing.append(("segment_index", "INTEGER DEFAULT 0"))
    for name, col_type in missing:
        await conn.execute(
            text(f"ALTER TABLE scheduled_occurrences ADD COLUMN {name} {col_type}")
        )


async def _ensure_sqlite_recurrence_columns(conn) -> None:
    result = await conn.execute(text("PRAGMA table_info(recurrences)"))
    columns = {row[1] for row in result.fetchall()}
    missing = []
    if "created_at" not in columns:
        missing.append(("created_at", "DATETIME"))
    if "updated_at" not in columns:
        missing.append(("updated_at", "DATETIME"))
    for name, col_type in missing:
        await conn.execute(text(f"ALTER TABLE recurrences ADD COLUMN {name} {col_type}"))
    if missing:
        await conn.execute(
            text(
                "UPDATE recurrences "
                "SET created_at = COALESCE(created_at, CURRENT_TIMESTAMP), "
                "updated_at = COALESCE(updated_at, CURRENT_TIMESTAMP)"
            )
        )
