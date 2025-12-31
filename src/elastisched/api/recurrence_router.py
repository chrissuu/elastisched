import uuid
from datetime import datetime, timedelta, timezone
from zoneinfo import ZoneInfo

from fastapi import APIRouter, Depends, HTTPException, Query, status
from sqlalchemy import select
from sqlalchemy.ext.asyncio import AsyncSession

from elastisched.api.db import get_session
from elastisched.api.models import RecurrenceModel
from elastisched.api.schemas import (
    OccurrenceRead,
    RecurrenceCreate,
    RecurrenceRead,
    RecurrenceUpdate,
    TimeRangeSchema,
)
from elastisched.blob import Blob
from elastisched.recurrence import (
    DateBlobRecurrence,
    DeltaBlobRecurrence,
    SingleBlobOccurrence,
    WeeklyBlobRecurrence,
)
from elastisched.timerange import TimeRange


recurrence_router = APIRouter(prefix="/recurrences", tags=["recurrences"])
occurrence_router = APIRouter(prefix="/occurrences", tags=["occurrences"])


def _parse_datetime(value: str) -> datetime:
    if isinstance(value, datetime):
        return value
    if not value:
        raise HTTPException(
            status_code=status.HTTP_422_UNPROCESSABLE_ENTITY,
            detail="Missing datetime in recurrence payload",
        )
    if isinstance(value, str) and value.endswith("Z"):
        value = value.replace("Z", "+00:00")
    try:
        return datetime.fromisoformat(value)
    except ValueError as exc:
        raise HTTPException(
            status_code=status.HTTP_422_UNPROCESSABLE_ENTITY,
            detail="Invalid datetime format in recurrence payload",
        ) from exc


def _parse_timerange(data: dict, tzinfo) -> TimeRange:
    start = _parse_datetime(data.get("start"))
    end = _parse_datetime(data.get("end"))
    if tzinfo:
        if start.tzinfo is None:
            start = start.replace(tzinfo=tzinfo)
        else:
            start = start.astimezone(tzinfo)
        if end.tzinfo is None:
            end = end.replace(tzinfo=tzinfo)
        else:
            end = end.astimezone(tzinfo)
    return TimeRange(start=start, end=end)


def _blob_from_payload(data: dict) -> Blob:
    tz_name = data.get("tz") or "UTC"
    try:
        tzinfo = ZoneInfo(tz_name)
    except Exception:
        tzinfo = timezone.utc
    default_tr = _parse_timerange(data.get("default_scheduled_timerange", {}), tzinfo)
    schedulable_tr = _parse_timerange(data.get("schedulable_timerange", {}), tzinfo)
    return Blob(
        default_scheduled_timerange=default_tr,
        schedulable_timerange=schedulable_tr,
        name=data.get("name") or "Unnamed Blob",
        description=data.get("description"),
        tz=tzinfo,
        policy=data.get("policy") or {},
        dependencies=set(data.get("dependencies") or []),
        tags=set(data.get("tags") or []),
    )


def _recurrence_from_payload(recurrence_type: str, payload: dict):
    payload = payload or {}
    if recurrence_type == "single":
        blob = _blob_from_payload(payload.get("blob") or {})
        return SingleBlobOccurrence(blob=blob)
    if recurrence_type == "weekly":
        interval = int(payload.get("interval") or 1)
        blobs = [_blob_from_payload(blob) for blob in payload.get("blobs_of_week") or []]
        if not blobs:
            raise HTTPException(
                status_code=status.HTTP_422_UNPROCESSABLE_ENTITY,
                detail="Weekly recurrence requires blobs_of_week",
            )
        return WeeklyBlobRecurrence(blobs_of_week=blobs, interval=interval)
    if recurrence_type == "delta":
        delta_seconds = payload.get("delta_seconds")
        if delta_seconds is None:
            raise HTTPException(
                status_code=status.HTTP_422_UNPROCESSABLE_ENTITY,
                detail="Delta recurrence requires delta_seconds",
            )
        start_blob = _blob_from_payload(payload.get("start_blob") or {})
        return DeltaBlobRecurrence(
            delta=timedelta(seconds=float(delta_seconds)),
            start_blob=start_blob,
        )
    if recurrence_type == "date":
        blob = _blob_from_payload(payload.get("blob") or {})
        return DateBlobRecurrence(blob=blob)
    raise HTTPException(
        status_code=status.HTTP_422_UNPROCESSABLE_ENTITY,
        detail="Unsupported recurrence type",
    )


def _occurrence_id(recurrence_id: str, blob: Blob) -> str:
    start = blob.get_schedulable_timerange().start
    return f"{recurrence_id}:{start.isoformat()}"


def _recurrence_tzinfo(recurrence_obj):
    if isinstance(recurrence_obj, WeeklyBlobRecurrence):
        base_start = recurrence_obj.blobs_of_week[0].get_schedulable_timerange().start
        return base_start.tzinfo
    if isinstance(recurrence_obj, DeltaBlobRecurrence):
        return recurrence_obj.start_blob.get_schedulable_timerange().start.tzinfo
    if isinstance(recurrence_obj, DateBlobRecurrence):
        return recurrence_obj.blob.get_schedulable_timerange().start.tzinfo
    if isinstance(recurrence_obj, SingleBlobOccurrence):
        return recurrence_obj.blob.get_schedulable_timerange().start.tzinfo
    return None


def _coerce_timerange(timerange: TimeRange, tzinfo) -> TimeRange:
    if tzinfo is None:
        return timerange
    start = timerange.start
    end = timerange.end
    if start.tzinfo:
        start = start.astimezone(tzinfo)
    else:
        start = start.replace(tzinfo=tzinfo)
    if end.tzinfo:
        end = end.astimezone(tzinfo)
    else:
        end = end.replace(tzinfo=tzinfo)
    return TimeRange(start=start, end=end)


def _to_occurrence_schema(
    recurrence_id: str, recurrence_type: str, payload: dict, blob: Blob
) -> OccurrenceRead:
    default_tr = blob.get_default_scheduled_timerange()
    schedulable_tr = blob.get_schedulable_timerange()
    return OccurrenceRead(
        id=_occurrence_id(recurrence_id, blob),
        recurrence_id=recurrence_id,
        recurrence_type=recurrence_type,
        recurrence_payload=payload,
        name=blob.name,
        description=blob.description,
        default_scheduled_timerange=TimeRangeSchema(
            start=default_tr.start, end=default_tr.end
        ),
        schedulable_timerange=TimeRangeSchema(
            start=schedulable_tr.start, end=schedulable_tr.end
        ),
        realized_timerange=None,
        tz=blob.tz.key if hasattr(blob.tz, "key") else str(blob.tz),
        policy=blob.policy or {},
        dependencies=list(blob.dependencies or []),
        tags=list(blob.tags or []),
    )


@recurrence_router.post("", response_model=RecurrenceRead, status_code=status.HTTP_201_CREATED)
async def create_recurrence(
    payload: RecurrenceCreate, session: AsyncSession = Depends(get_session)
) -> RecurrenceRead:
    _recurrence_from_payload(payload.type, payload.payload)
    recurrence = RecurrenceModel(
        id=str(uuid.uuid4()),
        type=payload.type,
        payload=payload.payload,
    )
    session.add(recurrence)
    await session.commit()
    await session.refresh(recurrence)
    return RecurrenceRead(id=recurrence.id, type=recurrence.type, payload=recurrence.payload)


@recurrence_router.get("", response_model=list[RecurrenceRead])
async def list_recurrences(
    session: AsyncSession = Depends(get_session),
) -> list[RecurrenceRead]:
    result = await session.execute(select(RecurrenceModel))
    return [
        RecurrenceRead(id=item.id, type=item.type, payload=item.payload)
        for item in result.scalars().all()
    ]


@recurrence_router.get("/{recurrence_id}", response_model=RecurrenceRead)
async def get_recurrence(
    recurrence_id: str, session: AsyncSession = Depends(get_session)
) -> RecurrenceRead:
    result = await session.execute(
        select(RecurrenceModel).where(RecurrenceModel.id == recurrence_id)
    )
    recurrence = result.scalar_one_or_none()
    if not recurrence:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="Recurrence not found")
    return RecurrenceRead(id=recurrence.id, type=recurrence.type, payload=recurrence.payload)


@recurrence_router.put("/{recurrence_id}", response_model=RecurrenceRead)
async def update_recurrence(
    recurrence_id: str, payload: RecurrenceUpdate, session: AsyncSession = Depends(get_session)
) -> RecurrenceRead:
    result = await session.execute(
        select(RecurrenceModel).where(RecurrenceModel.id == recurrence_id)
    )
    recurrence = result.scalar_one_or_none()
    if not recurrence:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="Recurrence not found")

    new_type = payload.type or recurrence.type
    new_payload = payload.payload if payload.payload is not None else recurrence.payload
    _recurrence_from_payload(new_type, new_payload)
    recurrence.type = new_type
    recurrence.payload = new_payload
    await session.commit()
    await session.refresh(recurrence)
    return RecurrenceRead(id=recurrence.id, type=recurrence.type, payload=recurrence.payload)


@recurrence_router.delete("/{recurrence_id}", status_code=status.HTTP_204_NO_CONTENT)
async def delete_recurrence(
    recurrence_id: str, session: AsyncSession = Depends(get_session)
) -> None:
    result = await session.execute(
        select(RecurrenceModel).where(RecurrenceModel.id == recurrence_id)
    )
    recurrence = result.scalar_one_or_none()
    if not recurrence:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="Recurrence not found")
    await session.delete(recurrence)
    await session.commit()


@occurrence_router.get("", response_model=list[OccurrenceRead])
async def list_occurrences(
    start: datetime = Query(..., description="Range start"),
    end: datetime = Query(..., description="Range end"),
    session: AsyncSession = Depends(get_session),
) -> list[OccurrenceRead]:
    if start.tzinfo and not end.tzinfo:
        end = end.replace(tzinfo=start.tzinfo)
    elif end.tzinfo and not start.tzinfo:
        start = start.replace(tzinfo=end.tzinfo)
    elif start.tzinfo and end.tzinfo and start.tzinfo != end.tzinfo:
        end = end.astimezone(start.tzinfo)
    timerange = TimeRange(start=start, end=end)
    result = await session.execute(select(RecurrenceModel))
    occurrences: list[OccurrenceRead] = []
    for recurrence in result.scalars().all():
        recurrence_obj = _recurrence_from_payload(recurrence.type, recurrence.payload)
        recurrence_tz = _recurrence_tzinfo(recurrence_obj)
        recurrence_range = _coerce_timerange(timerange, recurrence_tz)
        for blob in recurrence_obj.all_occurrences(recurrence_range):
            occurrences.append(
                _to_occurrence_schema(
                    recurrence.id, recurrence.type, recurrence.payload, blob
                )
            )
    occurrences.sort(key=lambda item: item.default_scheduled_timerange.start)
    return occurrences
