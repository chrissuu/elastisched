import os


DEFAULT_DATABASE_URL = "sqlite+aiosqlite:///./core.db"


def get_database_url() -> str:
    return os.getenv("DATABASE_URL", DEFAULT_DATABASE_URL)
