from enum import Enum
from dataclasses import dataclass, field
from datetime import datetime, timedelta

class Day(Enum):
    MONDAY = 0
    TUESDAY = 1
    WEDNESDAY = 2
    THURSDAY = 3
    FRIDAY = 4
    SATURDAY = 5
    SUNDAY = 6

@dataclass
class TimeRange:
    """Represents a time range with start and end times"""
    start: datetime
    end: datetime
    
    def __post_init__(self):
        if self.start >= self.end:
            raise ValueError("Start time must be before end time")
    
    def duration(self) -> timedelta:
        return self.end - self.start
    
    def overlaps(self, other: 'TimeRange') -> bool:
        return self.start < other.end and other.start < self.end
    
    def contains(self, other: 'TimeRange') -> bool:
        return self.start <= other.start and other.end <= self.end