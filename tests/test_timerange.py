from elastisched.timerange import TimeRange
from datetime import datetime, timezone, timedelta
import pytest

def test_default_timerange_is_utc():
    # Given
    timerange = TimeRange()

    # Assert that
    assert timerange.start.tzinfo == timezone.utc
    assert timerange.end.tzinfo == timezone.utc

def test_different_timezone_equal_comparison_raise_value_error():
    # Given
    timerange_utc = TimeRange()
    timerange_ot = TimeRange(
        start=datetime(year=2000, month=1, day=1, tzinfo=timezone(timedelta(hours=-5))),
        end=datetime(year=2000, month=1, day=2, tzinfo=timezone(timedelta(hours=-5)))
    )

    # Assert that
    with pytest.raises(ValueError):
        timerange_utc == timerange_ot

def test_different_timezone_le_comparison_raise_value_error():
    # Given
    timerange_utc = TimeRange()
    timerange_ot = TimeRange(
        start=datetime(year=2000, month=1, day=1, tzinfo=timezone(timedelta(hours=-5))),
        end=datetime(year=2000, month=1, day=2, tzinfo=timezone(timedelta(hours=-5)))
    )

    # Assert that
    with pytest.raises(ValueError):
        timerange_utc <= timerange_ot