import os


DEFAULT_DATABASE_URL = "sqlite+aiosqlite:///./core.db"
DEFAULT_GEMINI_MODEL = "gemini-3-flash-preview"
DEFAULT_MAX_BLOB_CREATION_RETRIES = 2


def get_database_url() -> str:
    return os.getenv("DATABASE_URL", DEFAULT_DATABASE_URL)


def get_gemini_api_key() -> str:
    return os.getenv("GEMINI_API_KEY", "")


def get_gemini_model() -> str:
    return os.getenv("GEMINI_MODEL", DEFAULT_GEMINI_MODEL)


def get_max_blob_creation_retries() -> int:
    raw = os.getenv("MAX_BLOB_CREATION_RETRIES", str(DEFAULT_MAX_BLOB_CREATION_RETRIES))
    try:
        value = int(raw)
    except ValueError:
        return DEFAULT_MAX_BLOB_CREATION_RETRIES
    return max(0, value)
