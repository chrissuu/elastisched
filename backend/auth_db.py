from typing import AsyncGenerator

from sqlalchemy.ext.asyncio import AsyncSession, async_sessionmaker, create_async_engine

from backend.auth_models import AuthBase
from backend.config import get_auth_database_url


AUTH_DATABASE_URL = get_auth_database_url()

auth_engine = create_async_engine(AUTH_DATABASE_URL, pool_pre_ping=True)
AuthSessionLocal = async_sessionmaker(auth_engine, expire_on_commit=False)


async def get_auth_session() -> AsyncGenerator[AsyncSession, None]:
    async with AuthSessionLocal() as session:
        yield session


async def init_auth_db() -> None:
    import backend.auth_models as auth_models  # noqa: F401

    metadata = AuthBase.metadata
    if not metadata.tables:
        metadata = auth_models.AuthBase.metadata
    async with auth_engine.begin() as conn:
        await conn.run_sync(metadata.create_all)
