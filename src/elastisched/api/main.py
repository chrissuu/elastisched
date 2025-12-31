from fastapi import FastAPI

from elastisched.api.db import init_db
from elastisched.api.router import router as blob_router


app = FastAPI(title="Elastisched API")


@app.on_event("startup")
async def on_startup() -> None:
    await init_db()


@app.get("/health")
async def health() -> dict:
    return {"status": "ok"}


app.include_router(blob_router)
