from datetime import datetime
from typing import Any

from fastapi import HTTPException
from mcp.server.fastmcp import FastMCP

from elastisched_api import recurrence_router as recurrence_router_module
from elastisched_api import router as blob_router_module
from elastisched_api import schedule_router as schedule_router_module
from elastisched_api.db import AsyncSessionLocal
from elastisched_api.schemas import (
    BlobCreate,
    BlobRead,
    BlobUpdate,
    OccurrenceRead,
    RecurrenceCreate,
    RecurrenceRead,
    RecurrenceUpdate,
    ScheduleRequest,
    ScheduleResponse,
    ScheduleStatus,
)


mcp = FastMCP("Elastisched MCP")


async def _call_with_session(func, *args, **kwargs):
    async with AsyncSessionLocal() as session:
        try:
            return await func(*args, session=session, **kwargs)
        except HTTPException as exc:
            detail = exc.detail if isinstance(exc.detail, str) else str(exc.detail)
            raise ValueError(detail) from exc


@mcp.tool()
async def list_blobs(
    overlaps_start: datetime | None = None,
    overlaps_end: datetime | None = None,
) -> list[BlobRead]:
    """List blobs, optionally filtered by overlapping schedulable ranges."""
    return await _call_with_session(
        blob_router_module.list_blobs,
        overlaps_start=overlaps_start,
        overlaps_end=overlaps_end,
    )


@mcp.tool()
async def create_blob(payload: BlobCreate) -> BlobRead:
    """Create a new blob."""
    return await _call_with_session(blob_router_module.create_blob, payload)


@mcp.tool()
async def update_blob(blob_id: str, payload: BlobUpdate) -> BlobRead:
    """Update an existing blob."""
    return await _call_with_session(
        blob_router_module.update_blob, blob_id=blob_id, payload=payload
    )


@mcp.tool()
async def delete_blob(blob_id: str) -> dict[str, Any]:
    """Delete a blob."""
    await _call_with_session(blob_router_module.delete_blob, blob_id=blob_id)
    return {"deleted": True, "blob_id": blob_id}


@mcp.tool()
async def list_recurrences() -> list[RecurrenceRead]:
    """List recurrence definitions."""
    return await _call_with_session(recurrence_router_module.list_recurrences)


@mcp.tool()
async def create_recurrence(payload: RecurrenceCreate) -> RecurrenceRead:
    """Create a recurrence definition."""
    return await _call_with_session(recurrence_router_module.create_recurrence, payload)


@mcp.tool()
async def update_recurrence(recurrence_id: str, payload: RecurrenceUpdate) -> RecurrenceRead:
    """Update an existing recurrence definition."""
    return await _call_with_session(
        recurrence_router_module.update_recurrence,
        recurrence_id=recurrence_id,
        payload=payload,
    )


@mcp.tool()
async def delete_recurrence(recurrence_id: str) -> dict[str, Any]:
    """Delete a recurrence definition."""
    await _call_with_session(
        recurrence_router_module.delete_recurrence, recurrence_id=recurrence_id
    )
    return {"deleted": True, "recurrence_id": recurrence_id}


@mcp.tool()
async def list_occurrences(start: datetime, end: datetime) -> list[OccurrenceRead]:
    """List occurrences between two datetimes."""
    return await _call_with_session(
        recurrence_router_module.list_occurrences, start=start, end=end
    )


@mcp.tool()
async def schedule_status() -> ScheduleStatus:
    """Get schedule status."""
    return await _call_with_session(schedule_router_module.get_schedule_status)


@mcp.tool()
async def run_schedule(payload: ScheduleRequest) -> ScheduleResponse:
    """Run the scheduler and return occurrences."""
    return await _call_with_session(schedule_router_module.run_schedule, payload)
