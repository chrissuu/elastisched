from datetime import datetime, timedelta, timezone

from core.constants import DEFAULT_TZ
from core.timerange import TimeRange


def test_default_timerange_uses_default_timezone():
    # Given
    timerange = TimeRange()

    # Assert that
    assert timerange.start.tzinfo == DEFAULT_TZ
    assert timerange.end.tzinfo == DEFAULT_TZ


def test_different_timezone_equal_comparison_normalizes_timezone():
    # Given
    timerange_utc = TimeRange(
        start=datetime(year=2000, month=1, day=1, tzinfo=DEFAULT_TZ),
        end=datetime(year=2000, month=1, day=2, tzinfo=DEFAULT_TZ),
    )
    timerange_ot = TimeRange(
        start=datetime(
            year=2000, month=1, day=2, hour=1, tzinfo=timezone(timedelta(hours=-5))
        ),
        end=datetime(
            year=2000, month=1, day=2, hour=2, tzinfo=timezone(timedelta(hours=-5))
        ),
    )

    # Assert that
    assert (timerange_utc == timerange_ot) is False


def test_different_timezone_le_comparison_normalizes_timezone():
    # Given
    timerange_utc = TimeRange(
        start=datetime(year=2000, month=1, day=1, tzinfo=DEFAULT_TZ),
        end=datetime(year=2000, month=1, day=2, tzinfo=DEFAULT_TZ),
    )
    timerange_ot = TimeRange(
        start=datetime(
            year=2000, month=1, day=2, hour=1, tzinfo=timezone(timedelta(hours=-5))
        ),
        end=datetime(
            year=2000, month=1, day=2, hour=2, tzinfo=timezone(timedelta(hours=-5))
        ),
    )

    # Assert that
    assert timerange_utc <= timerange_ot
