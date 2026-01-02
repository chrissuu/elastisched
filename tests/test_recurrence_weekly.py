from datetime import datetime

from elastisched.constants import DEFAULT_TZ
from elastisched.blob import Blob
from elastisched.recurrence import WeeklyBlobRecurrence
from elastisched.timerange import TimeRange
import pytest


def test_weekly_next_occurrence_same_week():
    # Given
    monday_blob = Blob(
        default_scheduled_timerange=TimeRange(
            start=datetime(year=2000, month=1, day=3, hour=9),
            end=datetime(year=2000, month=1, day=3, hour=10),
        ),
        schedulable_timerange=TimeRange(
            start=datetime(year=2000, month=1, day=3, hour=8),
            end=datetime(year=2000, month=1, day=3, hour=11),
        ),
    )
    wednesday_blob = Blob(
        default_scheduled_timerange=TimeRange(
            start=datetime(year=2000, month=1, day=5, hour=14),
            end=datetime(year=2000, month=1, day=5, hour=15),
        ),
        schedulable_timerange=TimeRange(
            start=datetime(year=2000, month=1, day=5, hour=13),
            end=datetime(year=2000, month=1, day=5, hour=16),
        ),
    )

    weekly_recurrence = WeeklyBlobRecurrence(
        blobs_of_week=[monday_blob, wednesday_blob]
    )

    current_time = datetime(
        year=2000, month=1, day=3, hour=10, minute=30, tzinfo=DEFAULT_TZ
    )
    next_occurrence = weekly_recurrence.next_occurrence(current_time)

    # Assert that
    assert next_occurrence is not None
    assert next_occurrence.get_schedulable_timerange().start.weekday() == 2
    assert next_occurrence.get_schedulable_timerange().start.hour == 13
    assert next_occurrence.get_default_scheduled_timerange().start.weekday() == 2
    assert next_occurrence.get_default_scheduled_timerange().start.hour == 14


def test_weekly_next_occurrence_next_week():
    # Given
    monday_blob = Blob(
        default_scheduled_timerange=TimeRange(
            start=datetime(year=2000, month=1, day=3, hour=9),
            end=datetime(year=2000, month=1, day=3, hour=10),
        ),
        schedulable_timerange=TimeRange(
            start=datetime(year=2000, month=1, day=3, hour=8),
            end=datetime(year=2000, month=1, day=3, hour=11),
        ),
    )
    wednesday_blob = Blob(
        default_scheduled_timerange=TimeRange(
            start=datetime(year=2000, month=1, day=5, hour=14),
            end=datetime(year=2000, month=1, day=5, hour=15),
        ),
        schedulable_timerange=TimeRange(
            start=datetime(year=2000, month=1, day=5, hour=13),
            end=datetime(year=2000, month=1, day=5, hour=16),
        ),
    )

    weekly_recurrence = WeeklyBlobRecurrence(
        blobs_of_week=[monday_blob, wednesday_blob]
    )

    current_time = datetime(year=2000, month=1, day=7, hour=15, tzinfo=DEFAULT_TZ)
    next_occurrence = weekly_recurrence.next_occurrence(current_time)

    # Assert that
    assert next_occurrence is not None
    assert next_occurrence.get_schedulable_timerange().start.weekday() == 0
    assert next_occurrence.get_schedulable_timerange().start.hour == 8

    expected_date = datetime(year=2000, month=1, day=10, hour=8, tzinfo=DEFAULT_TZ)
    assert next_occurrence.get_schedulable_timerange().start == expected_date


def test_weekly_next_occurrence_with_interval():
    # Given
    monday_blob = Blob(
        default_scheduled_timerange=TimeRange(
            start=datetime(year=2000, month=1, day=3, hour=9),
            end=datetime(year=2000, month=1, day=3, hour=10),
        ),
        schedulable_timerange=TimeRange(
            start=datetime(year=2000, month=1, day=3, hour=8),
            end=datetime(year=2000, month=1, day=3, hour=11),
        ),
    )

    weekly_recurrence = WeeklyBlobRecurrence(blobs_of_week=[monday_blob], interval=2)

    current_time = datetime(year=2000, month=1, day=7, hour=15, tzinfo=DEFAULT_TZ)
    next_occurrence = weekly_recurrence.next_occurrence(current_time)

    # Assert that
    assert next_occurrence is not None
    expected_date = datetime(year=2000, month=1, day=17, hour=8, tzinfo=DEFAULT_TZ)
    assert next_occurrence.get_schedulable_timerange().start == expected_date


def test_weekly_next_occurrence_exact_time_match():
    # Given
    monday_blob = Blob(
        default_scheduled_timerange=TimeRange(
            start=datetime(year=2000, month=1, day=3, hour=9),
            end=datetime(year=2000, month=1, day=3, hour=10),
        ),
        schedulable_timerange=TimeRange(
            start=datetime(year=2000, month=1, day=3, hour=8),
            end=datetime(year=2000, month=1, day=3, hour=11),
        ),
    )

    weekly_recurrence = WeeklyBlobRecurrence(blobs_of_week=[monday_blob])

    current_time = datetime(year=2000, month=1, day=3, hour=8, tzinfo=DEFAULT_TZ)
    next_occurrence = weekly_recurrence.next_occurrence(current_time)

    # Assert that
    assert next_occurrence is not None
    expected_date = datetime(year=2000, month=1, day=10, hour=8, tzinfo=DEFAULT_TZ)
    assert next_occurrence.get_schedulable_timerange().start == expected_date


def test_weekly_all_occurrences_single_blob():
    # Given
    monday_blob = Blob(
        default_scheduled_timerange=TimeRange(
            start=datetime(year=2000, month=1, day=3, hour=9),
            end=datetime(year=2000, month=1, day=3, hour=10),
        ),
        schedulable_timerange=TimeRange(
            start=datetime(year=2000, month=1, day=3, hour=8),
            end=datetime(year=2000, month=1, day=3, hour=11),
        ),
    )

    weekly_recurrence = WeeklyBlobRecurrence(blobs_of_week=[monday_blob])

    search_range = TimeRange(
        start=datetime(year=2000, month=1, day=1),
        end=datetime(year=2000, month=1, day=22),
    )

    occurrences = weekly_recurrence.all_occurrences(search_range)

    # Assert that
    assert len(occurrences) == 3
    expected_dates = [
        datetime(year=2000, month=1, day=3, hour=8, tzinfo=DEFAULT_TZ),
        datetime(year=2000, month=1, day=10, hour=8, tzinfo=DEFAULT_TZ),
        datetime(year=2000, month=1, day=17, hour=8, tzinfo=DEFAULT_TZ),
    ]
    for i, occurrence in enumerate(occurrences):
        assert occurrence.get_schedulable_timerange().start == expected_dates[i]


def test_weekly_all_occurrences_multiple_blobs():
    # Given
    monday_blob = Blob(
        default_scheduled_timerange=TimeRange(
            start=datetime(year=2000, month=1, day=3, hour=9),
            end=datetime(year=2000, month=1, day=3, hour=10),
        ),
        schedulable_timerange=TimeRange(
            start=datetime(year=2000, month=1, day=3, hour=8),
            end=datetime(year=2000, month=1, day=3, hour=11),
        ),
    )
    wednesday_blob = Blob(
        default_scheduled_timerange=TimeRange(
            start=datetime(year=2000, month=1, day=5, hour=14),
            end=datetime(year=2000, month=1, day=5, hour=15),
        ),
        schedulable_timerange=TimeRange(
            start=datetime(year=2000, month=1, day=5, hour=13),
            end=datetime(year=2000, month=1, day=5, hour=16),
        ),
    )

    weekly_recurrence = WeeklyBlobRecurrence(
        blobs_of_week=[monday_blob, wednesday_blob]
    )

    search_range = TimeRange(
        start=datetime(year=2000, month=1, day=1),
        end=datetime(year=2000, month=1, day=15),
    )
    occurrences = weekly_recurrence.all_occurrences(search_range)

    # Assert that
    assert len(occurrences) == 4

    assert occurrences[0].get_schedulable_timerange().start == datetime(
        year=2000, month=1, day=3, hour=8, tzinfo=DEFAULT_TZ
    )
    assert occurrences[1].get_schedulable_timerange().start == datetime(
        year=2000, month=1, day=5, hour=13, tzinfo=DEFAULT_TZ
    )
    assert occurrences[2].get_schedulable_timerange().start == datetime(
        year=2000, month=1, day=10, hour=8, tzinfo=DEFAULT_TZ
    )
    assert occurrences[3].get_schedulable_timerange().start == datetime(
        year=2000, month=1, day=12, hour=13, tzinfo=DEFAULT_TZ
    )


def test_weekly_all_occurrences_with_interval():
    # Given
    monday_blob = Blob(
        default_scheduled_timerange=TimeRange(
            start=datetime(year=2000, month=1, day=3, hour=9),
            end=datetime(year=2000, month=1, day=3, hour=10),
        ),
        schedulable_timerange=TimeRange(
            start=datetime(year=2000, month=1, day=3, hour=8),
            end=datetime(year=2000, month=1, day=3, hour=11),
        ),
    )

    weekly_recurrence = WeeklyBlobRecurrence(blobs_of_week=[monday_blob], interval=2)

    search_range = TimeRange(
        start=datetime(year=2000, month=1, day=1),
        end=datetime(year=2000, month=1, day=29),
    )
    occurrences = weekly_recurrence.all_occurrences(search_range)

    # Assert that
    assert len(occurrences) == 2
    assert occurrences[0].get_schedulable_timerange().start == datetime(
        year=2000, month=1, day=3, hour=8, tzinfo=DEFAULT_TZ
    )
    assert occurrences[1].get_schedulable_timerange().start == datetime(
        year=2000, month=1, day=17, hour=8, tzinfo=DEFAULT_TZ
    )


def test_weekly_all_occurrences_no_matches():
    # Given
    monday_blob = Blob(
        default_scheduled_timerange=TimeRange(
            start=datetime(year=2000, month=1, day=3, hour=9),
            end=datetime(year=2000, month=1, day=3, hour=10),
        ),
        schedulable_timerange=TimeRange(
            start=datetime(year=2000, month=1, day=3, hour=8),
            end=datetime(year=2000, month=1, day=3, hour=11),
        ),
    )

    weekly_recurrence = WeeklyBlobRecurrence(blobs_of_week=[monday_blob])

    search_range = TimeRange(
        start=datetime(year=1999, month=1, day=1),
        end=datetime(year=1999, month=12, day=31),
    )
    occurrences = weekly_recurrence.all_occurrences(search_range)

    # Assert that
    assert len(occurrences) == 0


def test_weekly_overlapping_blobs_validation():
    # Given
    blob1 = Blob(
        default_scheduled_timerange=TimeRange(
            start=datetime(year=2000, month=1, day=3, hour=9),
            end=datetime(year=2000, month=1, day=3, hour=11),
        ),
        schedulable_timerange=TimeRange(
            start=datetime(year=2000, month=1, day=3, hour=8),
            end=datetime(year=2000, month=1, day=3, hour=12),
        ),
    )
    blob2 = Blob(
        default_scheduled_timerange=TimeRange(
            start=datetime(year=2000, month=1, day=3, hour=10),
            end=datetime(year=2000, month=1, day=3, hour=12),
        ),
        schedulable_timerange=TimeRange(
            start=datetime(year=2000, month=1, day=3, hour=9),
            end=datetime(year=2000, month=1, day=3, hour=13),
        ),
    )

    # Assert that
    with pytest.raises(ValueError):
        WeeklyBlobRecurrence(blobs_of_week=[blob1, blob2])
