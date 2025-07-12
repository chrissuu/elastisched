from src.elastisched.utils import round_datetime_future_bias
import pytest
from datetime import datetime, timedelta, timezone

def test_round_to_future_minute_future_bias():
    """Test rounding to nearest minute with future bias."""
    dt = datetime(2024, 1, 1, 12, 30, 30, 500000)  # 30.5 seconds
    granularity = timedelta(minutes=1)
    result = round_datetime_future_bias(dt, granularity)
    expected = datetime(2024, 1, 1, 12, 31, 0)
    assert result == expected

def test_round_to_future_minute_already_rounded():
    """Test with datetime already at granularity boundary."""
    dt = datetime(2024, 1, 1, 12, 30, 0)
    granularity = timedelta(minutes=1)
    result = round_datetime_future_bias(dt, granularity)
    expected = datetime(2024, 1, 1, 12, 30, 0)
    assert result == expected

def test_round_to_future_minute_small_offset():
    """Test with small offset from boundary."""
    dt = datetime(2024, 1, 1, 12, 30, 0, 1)  # 1 microsecond past boundary
    granularity = timedelta(minutes=1)
    result = round_datetime_future_bias(dt, granularity)
    expected = datetime(2024, 1, 1, 12, 31, 0)
    assert result == expected

def test_round_to_future_hour_future_bias():
    """Test rounding to nearest hour with future bias."""
    dt = datetime(2024, 1, 1, 12, 30, 30)  # 30.5 minutes into hour
    granularity = timedelta(hours=1)
    result = round_datetime_future_bias(dt, granularity)
    expected = datetime(2024, 1, 1, 13, 0, 0)
    assert result == expected

def test_round_to_future_5_minutes():
    """Test rounding to 5-minute intervals."""
    dt = datetime(2024, 1, 1, 12, 32, 30)  # 2.5 minutes past 12:30
    granularity = timedelta(minutes=5)
    result = round_datetime_future_bias(dt, granularity)
    expected = datetime(2024, 1, 1, 12, 35, 0)
    assert result == expected

def test_round_to_future_15_minutes():
    """Test rounding to 15-minute intervals."""
    dt = datetime(2024, 1, 1, 12, 22, 30)  # 7.5 minutes past 12:15
    granularity = timedelta(minutes=15)
    result = round_datetime_future_bias(dt, granularity)
    expected = datetime(2024, 1, 1, 12, 30, 0)
    assert result == expected

def test_round_to_future_second():
    """Test rounding to nearest second."""
    dt = datetime(2024, 1, 1, 12, 30, 30, 500000)  # 30.5 seconds
    granularity = timedelta(seconds=1)
    result = round_datetime_future_bias(dt, granularity)
    expected = datetime(2024, 1, 1, 12, 30, 31)
    assert result == expected

def test_round_to_future_day():
    """Test rounding to nearest day."""
    dt = datetime(2024, 1, 1, 12, 30, 0)  # Noon
    granularity = timedelta(days=1)
    result = round_datetime_future_bias(dt, granularity)
    expected = datetime(2024, 1, 2, 0, 0, 0)
    assert result == expected


# Timezone-related tests
class TestTimezoneHandling:
    """Group timezone-related tests together."""
    
    def test_timezone_preservation_utc(self):
        """Test that timezone is preserved - UTC case."""
        dt = datetime(2024, 1, 1, 12, 30, 30, tzinfo=timezone.utc)
        granularity = timedelta(minutes=1)
        result = round_datetime_future_bias(dt, granularity)
        expected = datetime(2024, 1, 1, 12, 31, 0, tzinfo=timezone.utc)
        assert result == expected
        assert result.tzinfo == timezone.utc
    
    def test_timezone_preservation_custom(self):
        """Test that timezone is preserved - custom timezone case."""
        custom_tz = timezone(timedelta(hours=5))
        dt = datetime(2024, 1, 1, 12, 30, 30, tzinfo=custom_tz)
        granularity = timedelta(minutes=1)
        result = round_datetime_future_bias(dt, granularity)
        expected = datetime(2024, 1, 1, 12, 31, 0, tzinfo=custom_tz)
        assert result == expected
        assert result.tzinfo == custom_tz
    
    def test_naive_datetime(self):
        """Test with naive datetime (no timezone)."""
        dt = datetime(2024, 1, 1, 12, 30, 30)
        granularity = timedelta(minutes=1)
        result = round_datetime_future_bias(dt, granularity)
        expected = datetime(2024, 1, 1, 12, 31, 0)
        assert result == expected
        assert result.tzinfo is None


# Edge cases and precision tests
def test_exact_half_rounds_up():
    """Test that exactly halfway points round up (future bias)."""
    dt = datetime(2024, 1, 1, 12, 30, 30)  # Exactly 30 seconds
    granularity = timedelta(minutes=1)
    result = round_datetime_future_bias(dt, granularity)
    expected = datetime(2024, 1, 1, 12, 31, 0)
    assert result == expected

def test_microsecond_precision():
    """Test behavior with microsecond precision."""
    dt = datetime(2024, 1, 1, 12, 30, 30, 500000)  # 30.5 seconds
    granularity = timedelta(seconds=1)
    result = round_datetime_future_bias(dt, granularity)
    expected = datetime(2024, 1, 1, 12, 30, 31)
    assert result == expected

def test_very_small_granularity():
    """Test with very small granularity (milliseconds)."""
    dt = datetime(2024, 1, 1, 12, 30, 30, 500500)  # 500.5 milliseconds
    granularity = timedelta(milliseconds=1)
    result = round_datetime_future_bias(dt, granularity)
    expected = datetime(2024, 1, 1, 12, 30, 30, 501000)
    assert result == expected

def test_large_granularity():
    """Test with large granularity (weeks)."""
    dt = datetime(2024, 1, 4, 12, 0, 0)  # Thursday
    granularity = timedelta(weeks=1)
    result = round_datetime_future_bias(dt, granularity)
    # Should round to next week boundary
    expected = datetime(2024, 1, 11, 0, 0, 0)  # Following Monday
    assert result == expected


# Parametrized tests for edge cases
@pytest.mark.parametrize("dt, granularity, expected", [
    # Test various granularities
    (datetime(2024, 1, 1, 12, 30, 15), timedelta(seconds=30), datetime(2024, 1, 1, 12, 30, 30)),
    (datetime(2024, 1, 1, 12, 30, 45), timedelta(seconds=30), datetime(2024, 1, 1, 12, 31, 0)),
    (datetime(2024, 1, 1, 12, 7, 30), timedelta(minutes=15), datetime(2024, 1, 1, 12, 15, 0)),
    (datetime(2024, 1, 1, 12, 8, 0), timedelta(minutes=15), datetime(2024, 1, 1, 12, 15, 0)),
])
def test_parametrized_rounding(dt, granularity, expected):
    """Parametrized tests for various rounding scenarios."""
    result = round_datetime_future_bias(dt, granularity)
    assert result == expected


def test_edge_case_zero_granularity():
    """Test error handling for zero granularity."""
    dt = datetime(2024, 1, 1, 12, 30, 30)
    granularity = timedelta(0)
    
    with pytest.raises(ValueError):
        round_datetime_future_bias(dt, granularity)


def test_negative_granularity():
    """Test behavior with negative granularity."""
    dt = datetime(2024, 1, 1, 12, 30, 30)
    granularity = timedelta(minutes=-1)
    
    # This should handle negative granularity gracefully or raise an error
    # Adjust expectation based on desired behavior
    with pytest.raises(ValueError):
        round_datetime_future_bias(dt, granularity)
