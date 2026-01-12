from datetime import datetime, timedelta
from zoneinfo import ZoneInfo

from elastisched.blob import Blob
from elastisched.constants import DEFAULT_TZ
from elastisched.recurrence import DeltaBlobRecurrence
from elastisched.timerange import TimeRange


def test_delta_next_occurrence_before_start():
    # Given
    default_timerange = TimeRange(
        start=datetime(2000, 1, 1, 9),
        end=datetime(2000, 1, 1, 10),
    )
    schedulable_timerange = TimeRange(
        start=datetime(2000, 1, 1, 8),
        end=datetime(2000, 1, 1, 11),
    )
    blob = Blob(default_timerange, schedulable_timerange)
    recurrence = DeltaBlobRecurrence(delta=timedelta(days=1), start_blob=blob)

    dt = datetime(1999, 12, 30, 12, tzinfo=DEFAULT_TZ)
    next_occurrence = recurrence.next_occurrence(dt)

    # Assert that
    assert next_occurrence.default_scheduled_timerange == default_timerange
    assert next_occurrence.schedulable_timerange == schedulable_timerange


def test_delta_next_occurrence_after_start():
    # Given
    default_timerange = TimeRange(
        start=datetime(2000, 1, 1, 9),
        end=datetime(2000, 1, 1, 10),
    )
    schedulable_timerange = TimeRange(
        start=datetime(2000, 1, 1, 8),
        end=datetime(2000, 1, 1, 11),
    )
    blob = Blob(default_timerange, schedulable_timerange)
    recurrence = DeltaBlobRecurrence(delta=timedelta(days=1), start_blob=blob)

    dt = datetime(2000, 1, 3, 10, tzinfo=DEFAULT_TZ)
    next_occurrence = recurrence.next_occurrence(dt)

    # Assert that
    assert next_occurrence.get_schedulable_timerange().start == datetime(
        2000, 1, 4, 8, tzinfo=DEFAULT_TZ
    )
    assert next_occurrence.get_schedulable_timerange().end == datetime(
        2000, 1, 4, 11, tzinfo=DEFAULT_TZ
    )


def test_delta_all_occurrences_within_range():
    # Given
    default_timerange = TimeRange(
        start=datetime(2000, 1, 1, 9),
        end=datetime(2000, 1, 1, 10),
    )
    schedulable_timerange = TimeRange(
        start=datetime(2000, 1, 1, 8),
        end=datetime(2000, 1, 1, 11),
    )
    blob = Blob(default_timerange, schedulable_timerange)
    recurrence = DeltaBlobRecurrence(delta=timedelta(days=2), start_blob=blob)

    timerange = TimeRange(
        start=datetime(2000, 1, 1, 0),
        end=datetime(2000, 1, 8, 0),
    )
    occurrences = recurrence.all_occurrences(timerange)

    # Assert that
    expected_starts = [
        datetime(2000, 1, 1, 9, tzinfo=DEFAULT_TZ),
        datetime(2000, 1, 3, 9, tzinfo=DEFAULT_TZ),
        datetime(2000, 1, 5, 9, tzinfo=DEFAULT_TZ),
        datetime(2000, 1, 7, 9, tzinfo=DEFAULT_TZ),
    ]
    assert len(occurrences) == 4
    for i, expected_start in enumerate(expected_starts):
        assert occurrences[i].default_scheduled_timerange.start == expected_start


def test_delta_all_occurrences_none_in_range():
    # Given
    default_timerange = TimeRange(
        start=datetime(2000, 1, 1, 9),
        end=datetime(2000, 1, 1, 10),
    )
    schedulable_timerange = TimeRange(
        start=datetime(2000, 1, 1, 8),
        end=datetime(2000, 1, 1, 11),
    )
    blob = Blob(default_timerange, schedulable_timerange)
    recurrence = DeltaBlobRecurrence(delta=timedelta(days=1), start_blob=blob)

    timerange = TimeRange(
        start=datetime(1999, 1, 1),
        end=datetime(1999, 12, 1),
    )
    occurrences = recurrence.all_occurrences(timerange)

    # Assert that
    assert occurrences == []


def test_delta_next_occurrence_dst_agnostic(monkeypatch):
    tz = ZoneInfo("America/New_York")
    import elastisched.constants as constants
    import elastisched.timerange as timerange

    monkeypatch.setattr(constants, "DEFAULT_TZ", tz)
    monkeypatch.setattr(timerange, "DEFAULT_TZ", tz)

    default_timerange = TimeRange(
        start=datetime(2024, 3, 9, 9, tzinfo=tz),
        end=datetime(2024, 3, 9, 10, tzinfo=tz),
    )
    schedulable_timerange = TimeRange(
        start=datetime(2024, 3, 9, 8, tzinfo=tz),
        end=datetime(2024, 3, 9, 11, tzinfo=tz),
    )
    blob = Blob(default_timerange, schedulable_timerange, tz=tz)
    recurrence = DeltaBlobRecurrence(delta=timedelta(days=1), start_blob=blob)

    dt = datetime(2024, 3, 9, 12, tzinfo=tz)
    next_occurrence = recurrence.next_occurrence(dt)

    assert next_occurrence.get_schedulable_timerange().start == datetime(
        2024, 3, 10, 8, tzinfo=tz
    )
    assert next_occurrence.get_schedulable_timerange().end == datetime(
        2024, 3, 10, 11, tzinfo=tz
    )


def test_delta_next_occurrence_dst_fall_back_fold(monkeypatch):
    tz = ZoneInfo("America/New_York")
    import elastisched.constants as constants
    import elastisched.timerange as timerange

    monkeypatch.setattr(constants, "DEFAULT_TZ", tz)
    monkeypatch.setattr(timerange, "DEFAULT_TZ", tz)

    default_timerange = TimeRange(
        start=datetime(2024, 11, 2, 1, 30, tzinfo=tz),
        end=datetime(2024, 11, 2, 2, 30, tzinfo=tz),
    )
    schedulable_timerange = TimeRange(
        start=datetime(2024, 11, 2, 1, 30, tzinfo=tz),
        end=datetime(2024, 11, 2, 3, 30, tzinfo=tz),
    )
    blob = Blob(default_timerange, schedulable_timerange, tz=tz)
    recurrence = DeltaBlobRecurrence(delta=timedelta(days=1), start_blob=blob)

    dt = datetime(2024, 11, 3, 1, 0, tzinfo=tz, fold=1)
    next_occurrence = recurrence.next_occurrence(dt)

    assert next_occurrence.get_schedulable_timerange().start == datetime(
        2024, 11, 3, 1, 30, tzinfo=tz, fold=1
    )
    assert next_occurrence.get_schedulable_timerange().end == datetime(
        2024, 11, 3, 3, 30, tzinfo=tz, fold=1
    )


def test_delta_next_occurrence_dst_spring_forward_duration(monkeypatch):
    tz = ZoneInfo("America/New_York")
    import elastisched.constants as constants
    import elastisched.timerange as timerange

    monkeypatch.setattr(constants, "DEFAULT_TZ", tz)
    monkeypatch.setattr(timerange, "DEFAULT_TZ", tz)

    default_timerange = TimeRange(
        start=datetime(2024, 3, 9, 1, tzinfo=tz),
        end=datetime(2024, 3, 9, 9, tzinfo=tz),
    )
    schedulable_timerange = TimeRange(
        start=datetime(2024, 3, 9, 1, tzinfo=tz),
        end=datetime(2024, 3, 9, 9, tzinfo=tz),
    )
    blob = Blob(default_timerange, schedulable_timerange, tz=tz)
    recurrence = DeltaBlobRecurrence(delta=timedelta(days=1), start_blob=blob)

    dt = datetime(2024, 3, 9, 12, tzinfo=tz)
    next_occurrence = recurrence.next_occurrence(dt)

    assert next_occurrence.get_schedulable_timerange().start == datetime(
        2024, 3, 10, 1, tzinfo=tz
    )
    assert next_occurrence.get_schedulable_timerange().end == datetime(
        2024, 3, 10, 9, tzinfo=tz
    )
