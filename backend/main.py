from contextlib import asynccontextmanager
from pathlib import Path

from fastapi import FastAPI, Request
from fastapi.staticfiles import StaticFiles

from backend.auth_db import init_auth_db
from backend.auth_router import auth_router
from backend.analytics_router import analytics_router
from backend.analytics_db import init_analytics_db
from backend.db import init_db
from backend.integrations.router import integration_router
from backend.llm_router import llm_router
from backend.router import router as blob_router
from backend.recurrence_router import (
    occurrence_router,
    recurrence_router,
)
from backend.schedule_router import schedule_router


@asynccontextmanager
async def lifespan(_: FastAPI):
    await init_db()
    await init_auth_db()
    await init_analytics_db()
    yield


app = FastAPI(title="Elastisched API", lifespan=lifespan)
_UI_DIR = Path(__file__).resolve().parents[2] / "frontend"
_LANDING_DIR = Path(__file__).resolve().parents[2] / "landing"
if _UI_DIR.exists():
    app.mount("/ui", StaticFiles(directory=_UI_DIR, html=True), name="ui")


@app.middleware("http")
async def add_security_headers(request: Request, call_next):
    response = await call_next(request)
    response.headers.setdefault("X-Content-Type-Options", "nosniff")
    response.headers.setdefault("X-Frame-Options", "DENY")
    response.headers.setdefault("Referrer-Policy", "strict-origin-when-cross-origin")
    response.headers.setdefault("Permissions-Policy", "camera=(), microphone=(), geolocation=()")
    response.headers.setdefault("Cross-Origin-Opener-Policy", "same-origin")
    response.headers.setdefault("Cross-Origin-Resource-Policy", "same-origin")
    response.headers.setdefault("Cache-Control", "no-store")
    if request.url.scheme == "https":
        response.headers.setdefault(
            "Strict-Transport-Security",
            "max-age=31536000; includeSubDomains",
        )
    return response


@app.get("/health", operation_id="health_check")
async def health() -> dict:
    return {"status": "ok"}


app.include_router(auth_router)
app.include_router(blob_router)
app.include_router(llm_router)
app.include_router(recurrence_router)
app.include_router(occurrence_router)
app.include_router(schedule_router)
app.include_router(integration_router)
app.include_router(analytics_router)

if _LANDING_DIR.exists():
    app.mount("/", StaticFiles(directory=_LANDING_DIR, html=True), name="landing")
