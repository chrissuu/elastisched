from collections import deque
from datetime import datetime, timedelta, timezone
from zoneinfo import ZoneInfo

from fastapi import APIRouter, Depends, HTTPException, status
from sqlalchemy import delete, select
from sqlalchemy.ext.asyncio import AsyncSession

import engine
from elastisched_api.db import get_session
from elastisched_api.models import (
    RecurrenceModel,
    ScheduledOccurrenceModel,
    ScheduleStateModel,
)
from elastisched_api.recurrence_router import (
    _coerce_timerange,
    _exclusion_set,
    _payload_end_datetime,
    _parse_datetime,
    _recurrence_from_payload,
    _recurrence_tzinfo,
    _to_occurrence_schema,
)
from elastisched_api.schemas import (
    ScheduleRequest,
    ScheduleResponse,
    ScheduleStatus,
    TimeRangeSchema,
)
from elastisched.constants import DEFAULT_TZ
from elastisched.timerange import TimeRange


schedule_router = APIRouter(prefix="/schedule", tags=["schedule"])
DEFAULT_LOOKAHEAD_SECONDS = 14 * 24 * 60 * 60


def _as_utc(value: datetime) -> datetime:
    if value.tzinfo is None:
        value = value.replace(tzinfo=DEFAULT_TZ)
    return value.astimezone(timezone.utc)


def _resolve_user_tz(name: str | None):
    if not name:
        return DEFAULT_TZ
    try:
        return ZoneInfo(name)
    except Exception:
        return DEFAULT_TZ


def _occurrence_override(payload: dict, occurrence) -> dict | None:
    overrides = payload.get("occurrence_overrides") if isinstance(payload, dict) else None
    if not isinstance(overrides, dict):
        return None
    occurrence_start = occurrence.schedulable_timerange.start
    occurrence_ts = int(occurrence_start.timestamp())
    for key, value in overrides.items():
        if not isinstance(value, dict):
            continue
        try:
            key_dt = _parse_datetime(key)
        except HTTPException:
            continue
        if key_dt.tzinfo is None:
            key_dt = key_dt.replace(tzinfo=occurrence_start.tzinfo)
        else:
            key_dt = key_dt.astimezone(occurrence_start.tzinfo)
        if int(key_dt.timestamp()) == occurrence_ts:
            return value
    return None


def _override_finished_at(override: dict, tzinfo):
    if not isinstance(override, dict):
        return None
    raw = override.get("finished_at")
    if not raw:
        return None
    try:
        finished_at = _parse_datetime(raw)
    except HTTPException:
        return None
    if tzinfo:
        if finished_at.tzinfo is None:
            finished_at = finished_at.replace(tzinfo=tzinfo)
        else:
            finished_at = finished_at.astimezone(tzinfo)
    return finished_at


def _occurrence_effective_end(occurrence, override: dict | None):
    base_end = occurrence.default_scheduled_timerange.end
    tzinfo = base_end.tzinfo
    finished_at = _override_finished_at(override, tzinfo)
    if finished_at:
        return finished_at
    added_minutes = override.get("added_minutes") if isinstance(override, dict) else None
    try:
        added_minutes = float(added_minutes or 0)
    except (TypeError, ValueError):
        added_minutes = 0
    if added_minutes:
        return base_end + timedelta(minutes=added_minutes)
    return base_end


def _epoch_start_utc(reference_utc: datetime, user_tz) -> datetime:
    reference_local = _as_utc(reference_utc).astimezone(user_tz)
    start_local = reference_local - timedelta(days=reference_local.weekday())
    start_local = start_local.replace(hour=0, minute=0, second=0, microsecond=0)
    return start_local.astimezone(timezone.utc)


def _policy_from_payload(policy) -> engine.Policy:
    if isinstance(policy, engine.Policy):
        return policy
    if not isinstance(policy, dict):
        return engine.Policy(0, 0, 0)
    scheduling_policies = policy.get("scheduling_policies")
    if scheduling_policies is None:
        scheduling_policies = 0
        if policy.get("is_splittable"):
            scheduling_policies |= 1
        if policy.get("is_overlappable"):
            scheduling_policies |= 2
        if policy.get("is_invisible"):
            scheduling_policies |= 4
    max_splits = int(policy.get("max_splits") or 0)
    min_split_duration = int(
        policy.get("min_split_duration_seconds")
        or policy.get("min_split_duration")
        or 0
    )
    return engine.Policy(max_splits, min_split_duration, int(scheduling_policies))


def _tags_from_payload(raw_tags) -> set:
    tags = set()
    for tag in raw_tags or []:
        if isinstance(tag, engine.Tag):
            tags.add(tag)
            continue
        if isinstance(tag, dict):
            name = str(tag.get("name") or "").strip()
            if not name:
                continue
            description = str(tag.get("description") or "")
            tags.add(engine.Tag(name, description))
            continue
        if isinstance(tag, str):
            name = tag.strip()
            if not name:
                continue
            tags.add(engine.Tag(name))
    return tags


def _to_epoch_seconds(value_utc: datetime, epoch_start_utc: datetime) -> int:
    value_utc = _as_utc(value_utc)
    return int((value_utc - epoch_start_utc).total_seconds())


def _dependency_violation_message(jobs: list[engine.Job]) -> str | None:
    job_map = {job.id: job for job in jobs}
    in_degree = {job.id: 0 for job in jobs}
    adj_list = {job.id: [] for job in jobs}
    for job in jobs:
        for dep_id in job.dependencies:
            if dep_id in job_map:
                adj_list[dep_id].append(job.id)
                in_degree[job.id] += 1

    queue = deque([job_id for job_id, degree in in_degree.items() if degree == 0])
    topo_order = []
    while queue:
        current = queue.popleft()
        topo_order.append(current)
        for neighbor in adj_list[current]:
            in_degree[neighbor] -= 1
            if in_degree[neighbor] == 0:
                queue.append(neighbor)

    if len(topo_order) != len(jobs):
        return "Cyclic dependencies detected."

    processed = set()
    for job in jobs:
        for dep_id in job.dependencies:
            if dep_id in job_map and dep_id not in processed:
                return f"Dependency order violation for {job.id}."
        processed.add(job.id)
    return None


def _validate_schedule(schedule: engine.Schedule) -> str | None:
    jobs = list(schedule.scheduledJobs)
    for job in jobs:
        if not job.schedulableTimeRange.contains(job.scheduledTimeRange):
            return f"{job.id} scheduled outside schedulable window."

    non_overlappable = []
    for job in jobs:
        if job.policy.isOverlappable():
            continue
        for other in non_overlappable:
            if job.scheduledTimeRange.overlaps(other.scheduledTimeRange):
                return f"{job.id} overlaps with {other.id}."
        non_overlappable.append(job)

    return _dependency_violation_message(jobs)


async def _get_or_create_schedule_state(session: AsyncSession) -> ScheduleStateModel:
    result = await session.execute(select(ScheduleStateModel))
    state = result.scalar_one_or_none()
    if state is None:
        state = ScheduleStateModel(dirty=True)
        session.add(state)
        await session.commit()
        await session.refresh(state)
    return state


@schedule_router.get("/status", response_model=ScheduleStatus)
async def get_schedule_status(
    session: AsyncSession = Depends(get_session),
) -> ScheduleStatus:
    state = await _get_or_create_schedule_state(session)
    return ScheduleStatus(dirty=state.dirty, last_run=state.last_run)


@schedule_router.post("", response_model=ScheduleResponse)
async def run_schedule(
    payload: ScheduleRequest,
    session: AsyncSession = Depends(get_session),
) -> ScheduleResponse:
    lookahead_seconds = int(payload.lookahead_seconds or DEFAULT_LOOKAHEAD_SECONDS)
    if lookahead_seconds <= 0:
        raise HTTPException(
            status_code=status.HTTP_422_UNPROCESSABLE_ENTITY,
            detail="lookahead_seconds must be greater than 0.",
        )

    start_utc = datetime.now(timezone.utc)
    end_utc = start_utc + timedelta(seconds=lookahead_seconds)
    timerange = TimeRange(start=start_utc, end=end_utc)
    user_tz = _resolve_user_tz(payload.user_timezone)

    result = await session.execute(select(RecurrenceModel))
    occurrences = []
    for recurrence in result.scalars().all():
        exclusions = _exclusion_set(recurrence.payload or {})
        recurrence_obj = _recurrence_from_payload(recurrence.type, recurrence.payload)
        recurrence_tz = _recurrence_tzinfo(recurrence_obj)
        recurrence_range = _coerce_timerange(timerange, recurrence_tz)
        end_date = _payload_end_datetime(recurrence.payload or {}, recurrence_tz)
        if end_date and end_date < recurrence_range.start:
            continue
        if end_date and end_date < recurrence_range.end:
            recurrence_range = TimeRange(start=recurrence_range.start, end=end_date)
        for blob in recurrence_obj.all_occurrences(recurrence_range):
            if not recurrence_range.contains(blob.get_schedulable_timerange()):
                continue
            sched_start = blob.get_schedulable_timerange().start
            if end_date and sched_start > end_date:
                continue
            if sched_start.tzinfo is None:
                sched_start = sched_start.replace(tzinfo=timezone.utc)
            if int(sched_start.timestamp()) in exclusions:
                continue
            occurrences.append(
                _to_occurrence_schema(
                    recurrence.id, recurrence.type, recurrence.payload, blob
                )
            )

    epoch_start_utc = _epoch_start_utc(start_utc, user_tz)
    granularity_minutes = max(1, int(payload.granularity_minutes or 5))
    granularity_seconds = granularity_minutes * 60
    include_active = True if payload.include_active_occurrences is None else bool(
        payload.include_active_occurrences
    )
    now_utc = datetime.now(timezone.utc)

    jobs = []
    for occurrence in occurrences:
        override = _occurrence_override(occurrence.recurrence_payload or {}, occurrence)
        finished_at = _override_finished_at(
            override, occurrence.default_scheduled_timerange.end.tzinfo
        )
        if finished_at:
            continue
        if not include_active:
            effective_end = _occurrence_effective_end(occurrence, override)
            if _as_utc(occurrence.default_scheduled_timerange.start) <= now_utc < _as_utc(
                effective_end
            ):
                continue
        schedulable = occurrence.schedulable_timerange
        scheduled = occurrence.default_scheduled_timerange
        if schedulable.start >= schedulable.end:
            raise HTTPException(
                status_code=status.HTTP_422_UNPROCESSABLE_ENTITY,
                detail=f"Schedulable range invalid for {occurrence.id}.",
            )
        if scheduled.start >= scheduled.end:
            raise HTTPException(
                status_code=status.HTTP_422_UNPROCESSABLE_ENTITY,
                detail=f"Default scheduled range invalid for {occurrence.id}.",
            )
        schedulable_low = _to_epoch_seconds(schedulable.start, epoch_start_utc)
        schedulable_high = _to_epoch_seconds(schedulable.end, epoch_start_utc)
        scheduled_low = _to_epoch_seconds(scheduled.start, epoch_start_utc)
        scheduled_high = _to_epoch_seconds(scheduled.end, epoch_start_utc)
        duration = scheduled_high - scheduled_low
        if duration <= 0:
            raise HTTPException(
                status_code=status.HTTP_422_UNPROCESSABLE_ENTITY,
                detail=f"Non-positive duration for {occurrence.id}.",
            )
        job = engine.Job(
            duration,
            engine.TimeRange(schedulable_low, schedulable_high),
            engine.TimeRange(scheduled_low, scheduled_high),
            occurrence.id,
            _policy_from_payload(occurrence.policy),
            set(occurrence.dependencies or []),
            _tags_from_payload(occurrence.tags),
        )
        jobs.append(job)

    try:
        schedule = engine.schedule(jobs, granularity_seconds)
    except Exception as exc:
        raise HTTPException(
            status_code=status.HTTP_422_UNPROCESSABLE_ENTITY,
            detail=f"Scheduler failed: {exc}",
        ) from exc

    violation = _validate_schedule(schedule)
    if violation:
        raise HTTPException(
            status_code=status.HTTP_409_CONFLICT,
            detail=f"Scheduler could not find a valid schedule. {violation}",
        )

    await session.execute(delete(ScheduledOccurrenceModel))
    scheduled_rows = []
    for job in schedule.scheduledJobs:
        realized_start_utc = epoch_start_utc + timedelta(
            seconds=job.scheduledTimeRange.getLow()
        )
        realized_end_utc = epoch_start_utc + timedelta(
            seconds=job.scheduledTimeRange.getHigh()
        )
        realized_start = realized_start_utc.astimezone(DEFAULT_TZ)
        realized_end = realized_end_utc.astimezone(DEFAULT_TZ)
        scheduled_rows.append(
            ScheduledOccurrenceModel(
                id=job.id,
                realized_start=realized_start,
                realized_end=realized_end,
            )
        )
    if scheduled_rows:
        session.add_all(scheduled_rows)

    state = await _get_or_create_schedule_state(session)
    state.dirty = False
    state.last_run = datetime.now(timezone.utc)
    await session.commit()
    await session.refresh(state)

    scheduled_map = {row.id: row for row in scheduled_rows}
    if scheduled_map:
        occurrences = [
            occurrence.model_copy(
                update={
                    "realized_timerange": TimeRangeSchema(
                        start=scheduled_map[occurrence.id].realized_start,
                        end=scheduled_map[occurrence.id].realized_end,
                    )
                }
            )
            if occurrence.id in scheduled_map
            else occurrence
            for occurrence in occurrences
        ]

    return ScheduleResponse(
        occurrences=occurrences,
        dirty=state.dirty,
        last_run=state.last_run,
    )
