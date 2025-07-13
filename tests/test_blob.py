from elastisched.timerange import TimeRange
from datetime import datetime, timezone, timedelta
from elastisched.blob import Blob
import pytest


def test_different_timezone_le_comparison_raise_value_error():
    # Given
    default_timerange1 = TimeRange(
        start=datetime(year=2000, month=1, day=1, hour=1),
        end=datetime(year=2000, month=1, day=1, hour=12),
    )
    schedulable_timerange1 = TimeRange(
        start=datetime(year=2000, month=1, day=1, hour=1),
        end=datetime(year=2000, month=1, day=2, hour=23),
    )

    blob1 = Blob(
        default_scheduled_timerange=default_timerange1,
        schedulable_timerange=schedulable_timerange1,
    )

    default_timerange2 = TimeRange(
        start=datetime(
            year=2001, month=1, day=1, hour=1, tzinfo=timezone(timedelta(hours=-5))
        ),
        end=datetime(
            year=2001, month=1, day=1, hour=12, tzinfo=timezone(timedelta(hours=-5))
        ),
    )

    schedulable_timerange2 = TimeRange(
        start=datetime(
            year=2001, month=1, day=1, hour=1, tzinfo=timezone(timedelta(hours=-5))
        ),
        end=datetime(
            year=2001, month=1, day=2, hour=23, tzinfo=timezone(timedelta(hours=-5))
        ),
    )

    blob2 = Blob(
        default_scheduled_timerange=default_timerange2,
        schedulable_timerange=schedulable_timerange2,
    )

    # Assert that
    with pytest.raises(ValueError):
        blob1 <= blob2
