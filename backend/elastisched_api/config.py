import os


DEFAULT_DATABASE_URL = "sqlite+aiosqlite:///./core.db"
DEFAULT_GEMINI_MODEL = "gemini-3-flash-preview"


def get_database_url() -> str:
    return os.getenv("DATABASE_URL", DEFAULT_DATABASE_URL)


def get_gemini_api_key() -> str:
    return os.getenv("GEMINI_API_KEY", "")


def get_gemini_model() -> str:
    return os.getenv("GEMINI_MODEL", DEFAULT_GEMINI_MODEL)
