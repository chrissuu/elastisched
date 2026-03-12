import os
import secrets


DEFAULT_DATABASE_URL = "sqlite+aiosqlite:///./core.db"
DEFAULT_ANALYTICS_DATABASE_URL = "sqlite+aiosqlite:///./analytics.db"
DEFAULT_AUTH_DATABASE_URL = "sqlite+aiosqlite:///./auth.db"
DEFAULT_USER_WORKSPACE_DIR = "./workspaces"
DEFAULT_GEMINI_MODEL = "gemini-3-flash-preview"
DEFAULT_GEMINI_HTTP_TIMEOUT_SECONDS = 55.0
DEFAULT_LLM_RECURRENCE_DRAFT_TIMEOUT_SECONDS = 120.0
DEFAULT_MAX_BLOB_CREATION_RETRIES = 2
DEFAULT_PREFERENCE_BATCH_SIZE = 20
DEFAULT_SESSION_TTL_HOURS = 24 * 14
DEFAULT_AUTH_COOKIE_NAME = "elastisched_session"
DEFAULT_PASSWORD_MIN_LENGTH = 12
DEFAULT_GOOGLE_OAUTH_SCOPES = (
    "openid email profile https://www.googleapis.com/auth/calendar.readonly"
)
_RUNTIME_DEFAULT_SESSION_TOKEN_SECRET = secrets.token_urlsafe(48)


def get_database_url() -> str:
    return os.getenv("DATABASE_URL", DEFAULT_DATABASE_URL)


def get_analytics_database_url() -> str:
    return os.getenv("ANALYTICS_DATABASE_URL", DEFAULT_ANALYTICS_DATABASE_URL)


def get_auth_database_url() -> str:
    return os.getenv("AUTH_DATABASE_URL", DEFAULT_AUTH_DATABASE_URL)


def get_user_workspace_dir() -> str:
    raw = os.getenv("USER_WORKSPACE_DIR", DEFAULT_USER_WORKSPACE_DIR).strip()
    return raw or DEFAULT_USER_WORKSPACE_DIR


def get_session_ttl_hours() -> int:
    raw = os.getenv("SESSION_TTL_HOURS", str(DEFAULT_SESSION_TTL_HOURS)).strip()
    try:
        value = int(raw)
    except ValueError:
        return DEFAULT_SESSION_TTL_HOURS
    return max(1, value)


def get_auth_cookie_name() -> str:
    raw = os.getenv("AUTH_COOKIE_NAME", DEFAULT_AUTH_COOKIE_NAME).strip()
    return raw or DEFAULT_AUTH_COOKIE_NAME


def get_session_token_secret() -> str:
    raw = os.getenv("SESSION_TOKEN_SECRET", "").strip()
    if raw:
        return raw
    return _RUNTIME_DEFAULT_SESSION_TOKEN_SECRET


def get_password_pepper() -> str:
    return os.getenv("PASSWORD_PEPPER", "")


def get_password_min_length() -> int:
    raw = os.getenv("PASSWORD_MIN_LENGTH", str(DEFAULT_PASSWORD_MIN_LENGTH)).strip()
    try:
        value = int(raw)
    except ValueError:
        return DEFAULT_PASSWORD_MIN_LENGTH
    return max(8, min(128, value))


def get_gemini_api_key() -> str:
    return os.getenv("GEMINI_API_KEY", "")


def get_gemini_model() -> str:
    return os.getenv("GEMINI_MODEL", DEFAULT_GEMINI_MODEL)


def get_gemini_http_timeout_seconds() -> float:
    raw = os.getenv(
        "GEMINI_HTTP_TIMEOUT_SECONDS",
        str(DEFAULT_GEMINI_HTTP_TIMEOUT_SECONDS),
    )
    try:
        value = float(raw)
    except ValueError:
        return DEFAULT_GEMINI_HTTP_TIMEOUT_SECONDS
    return max(1.0, value)


def get_llm_recurrence_draft_timeout_seconds() -> float:
    raw = os.getenv(
        "LLM_RECURRENCE_DRAFT_TIMEOUT_SECONDS",
        str(DEFAULT_LLM_RECURRENCE_DRAFT_TIMEOUT_SECONDS),
    )
    try:
        value = float(raw)
    except ValueError:
        return DEFAULT_LLM_RECURRENCE_DRAFT_TIMEOUT_SECONDS
    return max(1.0, value)


def get_max_blob_creation_retries() -> int:
    raw = os.getenv("MAX_BLOB_CREATION_RETRIES", str(DEFAULT_MAX_BLOB_CREATION_RETRIES))
    try:
        value = int(raw)
    except ValueError:
        return DEFAULT_MAX_BLOB_CREATION_RETRIES
    return max(0, value)


def get_preference_batch_size() -> int:
    raw = os.getenv("PREFERENCE_BATCH_SIZE", str(DEFAULT_PREFERENCE_BATCH_SIZE))
    try:
        value = int(raw)
    except ValueError:
        return DEFAULT_PREFERENCE_BATCH_SIZE
    return max(1, value)


def get_google_oauth_client_id() -> str:
    return os.getenv("GOOGLE_OAUTH_CLIENT_ID", "")


def get_google_oauth_client_secret() -> str:
    return os.getenv("GOOGLE_OAUTH_CLIENT_SECRET", "")


def get_google_oauth_redirect_uri() -> str:
    return os.getenv("GOOGLE_OAUTH_REDIRECT_URI", "")


def get_google_oauth_scopes() -> str:
    raw = os.getenv("GOOGLE_OAUTH_SCOPES", DEFAULT_GOOGLE_OAUTH_SCOPES).strip()
    return raw or DEFAULT_GOOGLE_OAUTH_SCOPES
