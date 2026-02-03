from __future__ import annotations

from datetime import datetime

from backend.recurrence_router import (
    _coerce_timerange,
    _exclusion_set,
    _normalize_recurrence_type,
    _payload_end_datetime,
    _recurrence_from_payload,
    _recurrence_tzinfo,
    _to_occurrence_schema,
)
from backend.schemas import OccurrenceRead, RecurrenceCreate
from core.timerange import TimeRange


def build_preview_occurrences(
    recurrences: list[RecurrenceCreate],
    start: datetime,
    end: datetime,
) -> list[OccurrenceRead]:
    timerange = TimeRange(start=start, end=end)
    occurrences: list[OccurrenceRead] = []
    for index, recurrence in enumerate(recurrences):
        normalized_type = _normalize_recurrence_type(recurrence.type)
        payload = recurrence.payload or {}
        exclusions = _exclusion_set(payload)
        recurrence_obj = _recurrence_from_payload(normalized_type, payload)
        recurrence_tz = _recurrence_tzinfo(recurrence_obj)
        recurrence_range = _coerce_timerange(timerange, recurrence_tz)
        end_date = _payload_end_datetime(payload, recurrence_tz)
        if end_date and end_date < recurrence_range.start:
            continue
        if end_date and end_date < recurrence_range.end:
            recurrence_range = TimeRange(start=recurrence_range.start, end=end_date)
        for blob in recurrence_obj.all_occurrences(recurrence_range):
            sched_start = blob.get_schedulable_timerange().start
            if end_date and sched_start > end_date:
                continue
            if sched_start.tzinfo is None:
                sched_start = sched_start.replace(tzinfo=recurrence_range.start.tzinfo)
            if int(sched_start.timestamp()) in exclusions:
                continue
            occurrences.append(
                _to_occurrence_schema(
                    recurrence_id=f"draft-{index + 1}",
                    recurrence_type=normalized_type,
                    payload=payload,
                    blob=blob,
                )
            )
    occurrences.sort(key=lambda item: item.default_scheduled_timerange.start)
    return occurrences
