from datetime import datetime, timedelta

from elastisched.blob import Blob
from elastisched.constants import DEFAULT_TZ
from elastisched.recurrence import DateBlobRecurrence
from elastisched.timerange import TimeRange

import pytest


def test_dateblobrecurrence_regular_date_next_occurrence_before():
    # Given
    schedulable = TimeRange(
        start=datetime(2000, 5, 15, 10),
        end=datetime(2000, 5, 15, 12),
    )
    default = TimeRange(
        start=datetime(2000, 5, 15, 11),
        end=datetime(2000, 5, 15, 12),
    )
    blob = Blob(default, schedulable)
    recurrence = DateBlobRecurrence(blob=blob)

    dt = datetime(1999, 5, 1, tzinfo=DEFAULT_TZ)
    next_occurrence = recurrence.next_occurrence(dt)

    # Assert that
    assert next_occurrence.get_schedulable_timerange().start == datetime(
        2000, 5, 15, 10, tzinfo=DEFAULT_TZ
    )


def test_dateblobrecurrence_regular_date_next_occurrence_after():
    # Given
    schedulable = TimeRange(
        start=datetime(2000, 5, 15, 10),
        end=datetime(2000, 5, 15, 12),
    )
    default = TimeRange(
        start=datetime(2000, 5, 15, 11),
        end=datetime(2000, 5, 15, 12),
    )
    blob = Blob(default, schedulable)
    recurrence = DateBlobRecurrence(blob=blob)

    dt = datetime(2005, 5, 16, tzinfo=DEFAULT_TZ)
    next_occurrence = recurrence.next_occurrence(dt)

    # Assert that
    assert next_occurrence.get_schedulable_timerange().start == datetime(
        2006, 5, 15, 10, tzinfo=DEFAULT_TZ
    )


def test_dateblobrecurrence_all_occurrences_range():
    # Given
    schedulable = TimeRange(
        start=datetime(2000, 4, 10, 8),
        end=datetime(2000, 4, 10, 9),
    )
    default = TimeRange(
        start=datetime(2000, 4, 10, 8, 30),
        end=datetime(2000, 4, 10, 9),
    )
    blob = Blob(default, schedulable)
    recurrence = DateBlobRecurrence(blob=blob)

    timerange = TimeRange(
        start=datetime(2001, 1, 1),
        end=datetime(2003, 12, 31),
    )

    occurrences = recurrence.all_occurrences(timerange)

    expected_years = [2001, 2002, 2003]
    actual_years = [b.get_schedulable_timerange().start.year for b in occurrences]

    # Assert that
    assert actual_years == expected_years


def test_dateblobrecurrence_feb29_next_occurrence():
    # Given
    schedulable = TimeRange(
        start=datetime(2000, 2, 29, 10),
        end=datetime(2000, 2, 29, 12),
    )
    default = TimeRange(
        start=datetime(2000, 2, 29, 10),
        end=datetime(2000, 2, 29, 11),
    )
    blob = Blob(default, schedulable)
    recurrence = DateBlobRecurrence(blob=blob)

    dt = datetime(2001, 1, 1, tzinfo=DEFAULT_TZ)
    next_occurrence = recurrence.next_occurrence(dt)

    # Assert that
    assert next_occurrence.get_schedulable_timerange().start == datetime(
        2004, 2, 29, 10, tzinfo=DEFAULT_TZ
    )


def test_dateblobrecurrence_feb29_all_occurrences():
    # Given
    schedulable = TimeRange(
        start=datetime(2000, 2, 29, 10),
        end=datetime(2000, 2, 29, 12),
    )
    default = TimeRange(
        start=datetime(2000, 2, 29, 10),
        end=datetime(2000, 2, 29, 11),
    )
    blob = Blob(default, schedulable)
    recurrence = DateBlobRecurrence(blob=blob)

    timerange = TimeRange(
        start=datetime(2000, 1, 1),
        end=datetime(2020, 12, 31),
    )

    occurrences = recurrence.all_occurrences(timerange)
    expected_years = [2000, 2004, 2008, 2012, 2016, 2020]

    # Assert that
    actual_years = [b.get_schedulable_timerange().start.year for b in occurrences]
    assert actual_years == expected_years


def test_dateblobrecurrence_invalid_timerange_diff_weekdays():
    # Given
    schedulable = TimeRange(
        start=datetime(2000, 5, 15, 23),
        end=datetime(2000, 5, 16, 1),
    )
    default = TimeRange(
        start=datetime(2000, 5, 15, 23),
        end=datetime(2000, 5, 16, 0),
    )
    blob = Blob(default, schedulable)

    # Assert that
    with pytest.raises(ValueError, match="starts and ends on the same day"):
        DateBlobRecurrence(blob=blob)
