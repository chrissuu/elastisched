from elastisched.recurrence import SingleBlobOccurrence
from elastisched.blob import Blob
from elastisched.timerange import TimeRange
from datetime import datetime, timedelta, timezone

def test_next_occurrence():
    # Given
    default_timerange = TimeRange(
        start=datetime(year=2000, month=1, day=1, hour=1),
        end=datetime(year=2000, month=1, day=1, hour=12)
    )
    schedulable_timerange = TimeRange(
        start=datetime(year=2000, month=1, day=1, hour=1),
        end=datetime(year=2000, month=1, day=2, hour=23)
    )

    blob = Blob(
        default_scheduled_timerange=default_timerange,
        schedulable_timerange=schedulable_timerange
    )

    single_occurrence = SingleBlobOccurrence(blob)

    dt = datetime(year=1999, month=1, day=1, hour=1)
    next_occurrence = single_occurrence.next_occurrence(dt)

    # Assert that
    assert next_occurrence.default_scheduled_timerange == default_timerange
    assert next_occurrence.schedulable_timerange == schedulable_timerange