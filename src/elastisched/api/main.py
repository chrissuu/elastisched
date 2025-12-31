from pathlib import Path

from fastapi import FastAPI
from fastapi.staticfiles import StaticFiles

from elastisched.api.db import init_db
from elastisched.api.router import router as blob_router
from elastisched.api.recurrence_router import (
    occurrence_router,
    recurrence_router,
)


app = FastAPI(title="Elastisched API")
_UI_DIR = Path(__file__).resolve().parents[3] / "frontend"
if _UI_DIR.exists():
    app.mount("/ui", StaticFiles(directory=_UI_DIR, html=True), name="ui")


@app.on_event("startup")
async def on_startup() -> None:
    await init_db()


@app.get("/health")
async def health() -> dict:
    return {"status": "ok"}


app.include_router(blob_router)
app.include_router(recurrence_router)
app.include_router(occurrence_router)
