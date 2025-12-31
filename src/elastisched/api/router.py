import uuid
from datetime import datetime

from fastapi import APIRouter, Depends, HTTPException, Query, status
from sqlalchemy import select
from sqlalchemy.ext.asyncio import AsyncSession

from elastisched.api.db import get_session
from elastisched.api.models import BlobModel
from elastisched.api.schemas import BlobCreate, BlobRead, BlobUpdate, TimeRangeSchema


router = APIRouter(prefix="/blobs", tags=["blobs"])


def _validate_timeranges(
    default_tr: TimeRangeSchema, schedulable_tr: TimeRangeSchema
) -> None:
    if default_tr.start > default_tr.end:
        raise HTTPException(
            status_code=status.HTTP_422_UNPROCESSABLE_ENTITY,
            detail="default_scheduled_timerange.start must be before default_scheduled_timerange.end",
        )
    if schedulable_tr.start > schedulable_tr.end:
        raise HTTPException(
            status_code=status.HTTP_422_UNPROCESSABLE_ENTITY,
            detail="schedulable_timerange.start must be before schedulable_timerange.end",
        )
    if not (
        schedulable_tr.start <= default_tr.start <= default_tr.end <= schedulable_tr.end
    ):
        raise HTTPException(
            status_code=status.HTTP_422_UNPROCESSABLE_ENTITY,
            detail="default_scheduled_timerange must be within schedulable_timerange",
        )


def _to_schema(blob: BlobModel) -> BlobRead:
    return BlobRead(
        id=blob.id,
        name=blob.name,
        description=blob.description,
        default_scheduled_timerange=TimeRangeSchema(
            start=blob.default_scheduled_start, end=blob.default_scheduled_end
        ),
        schedulable_timerange=TimeRangeSchema(
            start=blob.schedulable_start, end=blob.schedulable_end
        ),
        tz=blob.tz,
        policy=blob.policy or {},
        dependencies=blob.dependencies or [],
        tags=blob.tags or [],
    )


@router.post("", response_model=BlobRead, status_code=status.HTTP_201_CREATED)
async def create_blob(
    payload: BlobCreate, session: AsyncSession = Depends(get_session)
) -> BlobRead:
    _validate_timeranges(payload.default_scheduled_timerange, payload.schedulable_timerange)
    blob = BlobModel(
        id=str(uuid.uuid4()),
        name=payload.name,
        description=payload.description,
        tz=payload.tz,
        default_scheduled_start=payload.default_scheduled_timerange.start,
        default_scheduled_end=payload.default_scheduled_timerange.end,
        schedulable_start=payload.schedulable_timerange.start,
        schedulable_end=payload.schedulable_timerange.end,
        policy=payload.policy,
        dependencies=payload.dependencies,
        tags=payload.tags,
    )
    session.add(blob)
    await session.commit()
    await session.refresh(blob)
    return _to_schema(blob)


@router.get("", response_model=list[BlobRead])
async def list_blobs(
    overlaps_start: datetime | None = Query(default=None),
    overlaps_end: datetime | None = Query(default=None),
    session: AsyncSession = Depends(get_session),
) -> list[BlobRead]:
    if (overlaps_start is None) != (overlaps_end is None):
        raise HTTPException(
            status_code=status.HTTP_422_UNPROCESSABLE_ENTITY,
            detail="Both overlaps_start and overlaps_end are required to filter by overlap",
        )
    query = select(BlobModel)
    if overlaps_start and overlaps_end:
        query = query.where(
            BlobModel.schedulable_start < overlaps_end,
            BlobModel.schedulable_end > overlaps_start,
        )
    result = await session.execute(query)
    return [_to_schema(blob) for blob in result.scalars().all()]


@router.get("/{blob_id}", response_model=BlobRead)
async def get_blob(
    blob_id: str, session: AsyncSession = Depends(get_session)
) -> BlobRead:
    result = await session.execute(select(BlobModel).where(BlobModel.id == blob_id))
    blob = result.scalar_one_or_none()
    if not blob:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="Blob not found")
    return _to_schema(blob)


@router.put("/{blob_id}", response_model=BlobRead)
async def update_blob(
    blob_id: str, payload: BlobUpdate, session: AsyncSession = Depends(get_session)
) -> BlobRead:
    result = await session.execute(select(BlobModel).where(BlobModel.id == blob_id))
    blob = result.scalar_one_or_none()
    if not blob:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="Blob not found")

    default_tr = payload.default_scheduled_timerange or TimeRangeSchema(
        start=blob.default_scheduled_start, end=blob.default_scheduled_end
    )
    schedulable_tr = payload.schedulable_timerange or TimeRangeSchema(
        start=blob.schedulable_start, end=blob.schedulable_end
    )
    _validate_timeranges(default_tr, schedulable_tr)

    if payload.name is not None:
        blob.name = payload.name
    if payload.description is not None:
        blob.description = payload.description
    if payload.tz is not None:
        blob.tz = payload.tz
    if payload.policy is not None:
        blob.policy = payload.policy
    if payload.dependencies is not None:
        blob.dependencies = payload.dependencies
    if payload.tags is not None:
        blob.tags = payload.tags
    blob.default_scheduled_start = default_tr.start
    blob.default_scheduled_end = default_tr.end
    blob.schedulable_start = schedulable_tr.start
    blob.schedulable_end = schedulable_tr.end

    await session.commit()
    await session.refresh(blob)
    return _to_schema(blob)


@router.delete("/{blob_id}", status_code=status.HTTP_204_NO_CONTENT)
async def delete_blob(
    blob_id: str, session: AsyncSession = Depends(get_session)
) -> None:
    result = await session.execute(select(BlobModel).where(BlobModel.id == blob_id))
    blob = result.scalar_one_or_none()
    if not blob:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="Blob not found")
    await session.delete(blob)
    await session.commit()
