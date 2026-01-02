from datetime import datetime, timedelta

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
