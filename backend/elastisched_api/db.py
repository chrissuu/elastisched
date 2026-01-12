from typing import AsyncGenerator
import importlib

from sqlalchemy import text
from sqlalchemy.ext.asyncio import AsyncSession, async_sessionmaker, create_async_engine
from sqlalchemy.orm import DeclarativeBase

from elastisched_api.config import get_database_url


DATABASE_URL = get_database_url()

engine = create_async_engine(DATABASE_URL, pool_pre_ping=True)
AsyncSessionLocal = async_sessionmaker(engine, expire_on_commit=False)


class Base(DeclarativeBase):
    pass


async def get_session() -> AsyncGenerator[AsyncSession, None]:
    async with AsyncSessionLocal() as session:
        yield session


async def init_db() -> None:
    # Ensure model metadata is registered on the current Base after reloads.
    import elastisched_api.models as models  # noqa: F401
    importlib.reload(models)
    async with engine.begin() as conn:
        await conn.run_sync(Base.metadata.create_all)
        if conn.dialect.name == "sqlite":
            await _ensure_sqlite_blob_columns(conn)
            await _ensure_sqlite_scheduled_occurrence_columns(conn)


async def _ensure_sqlite_blob_columns(conn) -> None:
    result = await conn.execute(text("PRAGMA table_info(blobs)"))
    columns = {row[1] for row in result.fetchall()}
    missing = []
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
