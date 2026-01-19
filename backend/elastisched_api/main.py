from pathlib import Path

from fastapi import FastAPI
from fastapi.staticfiles import StaticFiles

from elastisched_api.db import init_db
from elastisched_api.llm_router import llm_router
from elastisched_api.router import router as blob_router
from elastisched_api.recurrence_router import (
    occurrence_router,
    recurrence_router,
)
from elastisched_api.schedule_router import schedule_router


app = FastAPI(title="Elastisched API")
_UI_DIR = Path(__file__).resolve().parents[2] / "frontend"
if _UI_DIR.exists():
    app.mount("/ui", StaticFiles(directory=_UI_DIR, html=True), name="ui")


@app.on_event("startup")
async def on_startup() -> None:
    await init_db()


@app.get("/health", operation_id="health_check")
async def health() -> dict:
    return {"status": "ok"}


app.include_router(blob_router)
app.include_router(llm_router)
app.include_router(recurrence_router)
app.include_router(occurrence_router)
app.include_router(schedule_router)
