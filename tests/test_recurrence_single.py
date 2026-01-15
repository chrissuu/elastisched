from datetime import datetime, timedelta

from core.blob import Blob
from core.constants import DEFAULT_TZ
from core.recurrence import SingleBlobOccurrence
from core.timerange import TimeRange


def test_schedulable_after_current_next_occurrence():
    # Given
    default_timerange = TimeRange(
        start=datetime(year=2000, month=1, day=1, hour=1),
        end=datetime(year=2000, month=1, day=1, hour=12),
    )
    schedulable_timerange = TimeRange(
        start=datetime(year=2000, month=1, day=1, hour=1),
        end=datetime(year=2000, month=1, day=2, hour=23),
    )

    blob = Blob(
        default_scheduled_timerange=default_timerange,
        schedulable_timerange=schedulable_timerange,
    )

    single_occurrence = SingleBlobOccurrence(blob)

    dt = datetime(year=1999, month=1, day=1, hour=1, tzinfo=DEFAULT_TZ)
    next_occurrence = single_occurrence.next_occurrence(dt)

    # Assert that
    assert next_occurrence.default_scheduled_timerange == default_timerange
    assert next_occurrence.schedulable_timerange == schedulable_timerange


def test_schedulable_contains_current_next_occurrence():
    # Given
    default_timerange = TimeRange(
        start=datetime(year=2000, month=1, day=1, hour=1),
        end=datetime(year=2000, month=1, day=1, hour=12),
    )
    schedulable_timerange = TimeRange(
        start=datetime(year=1999, month=1, day=1, hour=1),
        end=datetime(year=2001, month=1, day=2, hour=23),
    )

    blob = Blob(
        default_scheduled_timerange=default_timerange,
        schedulable_timerange=schedulable_timerange,
    )

    single_occurrence = SingleBlobOccurrence(blob)

    dt = datetime(year=2000, month=6, day=1, hour=1, tzinfo=DEFAULT_TZ)
    next_occurrence = single_occurrence.next_occurrence(dt)

    # Assert that
    assert next_occurrence is None


def test_schedulable_before_current_next_occurrence():
    # Given
    default_timerange = TimeRange(
        start=datetime(year=2000, month=1, day=1, hour=1),
        end=datetime(year=2000, month=1, day=1, hour=12),
    )
    schedulable_timerange = TimeRange(
        start=datetime(year=1999, month=1, day=1, hour=1),
        end=datetime(year=2001, month=1, day=2, hour=23),
    )

    blob = Blob(
        default_scheduled_timerange=default_timerange,
        schedulable_timerange=schedulable_timerange,
    )

    single_occurrence = SingleBlobOccurrence(blob)

    dt = datetime(year=2005, month=6, day=1, hour=1, tzinfo=DEFAULT_TZ)
    next_occurrence = single_occurrence.next_occurrence(dt)

    # Assert that
    assert next_occurrence is None


def test_all_occurrences_includes_overlap():
    default_timerange = TimeRange(
        start=datetime(year=2000, month=1, day=1, hour=1),
        end=datetime(year=2000, month=1, day=3, hour=1),
    )
    schedulable_timerange = TimeRange(
        start=datetime(year=2000, month=1, day=1, hour=1),
        end=datetime(year=2000, month=1, day=3, hour=1),
    )
    blob = Blob(
        default_scheduled_timerange=default_timerange,
        schedulable_timerange=schedulable_timerange,
    )
    single_occurrence = SingleBlobOccurrence(blob)
    search_range = TimeRange(
        start=datetime(year=2000, month=1, day=2, hour=0),
        end=datetime(year=2000, month=1, day=2, hour=12),
    )

    occurrences = single_occurrence.all_occurrences(search_range)

    assert len(occurrences) == 1
    assert occurrences[0].get_schedulable_timerange() == schedulable_timerange
