from enum import Enum
from dataclasses import dataclass, field
from datetime import datetime, timedelta

DEFAULT_END_DATE = datetime(year=datetime.MAXYEAR, month=1, day=3)

class Day(Enum):
    MONDAY = 0
    TUESDAY = 1
    WEDNESDAY = 2
    THURSDAY = 3
    FRIDAY = 4
    SATURDAY = 5
    SUNDAY = 6

def round_datetime_future_bias(dt: datetime, granularity: timedelta) -> datetime:
    """
    Round datetime to nearest granularity with future bias.
    If exactly between two granularity points, round to the future one.
    """
    # Get total seconds for easier calculation
    total_seconds = dt.timestamp()
    granularity_seconds = granularity.total_seconds()
    
    # Calculate how many granularity units have passed
    units_passed = total_seconds / granularity_seconds
    
    # Round up (future bias) - use ceiling
    import math
    rounded_units = math.ceil(units_passed)
    
    # Convert back to datetime
    rounded_timestamp = rounded_units * granularity_seconds
    return datetime.fromtimestamp(rounded_timestamp, tz=dt.tzinfo)

@dataclass
class TimeRange:
    start: datetime
    end: datetime = field(default=DEFAULT_END_DATE)

    def __post_init__(self):
        if self.start >= self.end:
            raise ValueError("Start time must be before end time")
    
    def duration(self) -> timedelta:
        return self.end - self.start
    
    def overlaps(self, other: 'TimeRange') -> bool:
        return self.start < other.end and other.start < self.end
    
    def contains(self, other) -> bool:
        if isinstance(other, TimeRange):
            return self.start <= other.start and other.end <= self.end
        elif isinstance(other, datetime):
            return self.start <= other <= self.end
        else:
            raise TypeError("Passed argument should be of type TimeRange or datetime")
        
    def __eq__(self, other) -> bool:
        if not isinstance(other, TimeRange):
            return NotImplemented
        return self.start == other.start and self.end == other.end

    def __lt__(self, other) -> bool:
        """Less than: this range ends before other starts (non-overlapping)"""
        if not isinstance(other, TimeRange):
            return NotImplemented
        
        if self.overlaps(other):
            raise ValueError("lt comparison operator only applies to non-overlapping intervals")

        return self.end <= other.start 
    
    def __le__(self, other) -> bool:
        """Less than or equal"""
        if not isinstance(other, TimeRange):
            return NotImplemented

        if self.overlaps(other):
            raise ValueError("le comparison operator only applies to non-overlapping intervals")

        return self < other or self == other
    
    def __hash__(self) -> int:
        return hash((self.start, self.end))