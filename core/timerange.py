from dataclasses import dataclass, field
from datetime import datetime, timedelta, timezone
from functools import wraps
from typing import Tuple

from .constants import DEFAULT_END_DATE, DEFAULT_START_DATE, DEFAULT_TZ


def validate_timezone_compatibility(func):
    """Decorator to validate timezone compatibility between operators"""

    @wraps(func)
    def wrapper(self, other, *args, **kwargs):
        if self.start.tzinfo != other.start.tzinfo:
            raise ValueError("Cannot compare TimeRanges with different timezones")
        return func(self, other, *args, **kwargs)

    return wrapper


@dataclass
class TimeRange:
    start: datetime = field(default=DEFAULT_START_DATE)
    end: datetime = field(default=DEFAULT_END_DATE)

    def __post_init__(self):
        if self.start.tzinfo is None:
            self.start = self.start.replace(tzinfo=DEFAULT_TZ)
        else:
            self.start = self.start.astimezone(DEFAULT_TZ)

        if self.end.tzinfo is None:
            self.end = self.end.replace(tzinfo=DEFAULT_TZ)
        else:
            self.end = self.end.astimezone(DEFAULT_TZ)

        if self.start > self.end:
            raise ValueError(
                f"Start time <{self.start}> must be before or at end time <{self.end}>"
            )

    def duration(self) -> timedelta:
        return self.end - self.start

    @validate_timezone_compatibility
    def overlaps(self, other: "TimeRange") -> bool:
        if not isinstance(other, TimeRange):
            raise TypeError("Passed argument should be of type TimeRange")
        return self.start < other.end and other.start < self.end

    @validate_timezone_compatibility
    def contains(self, other) -> bool:
        if isinstance(other, TimeRange):
            return self.start <= other.start and other.end <= self.end
        elif isinstance(other, datetime):
            return self.start <= other <= self.end
        else:
            raise TypeError("Passed argument should be of type TimeRange or datetime")

    @validate_timezone_compatibility
    def __eq__(self, other) -> bool:
        if not isinstance(other, TimeRange):
            return NotImplemented
        return self.start == other.start and self.end == other.end

    @validate_timezone_compatibility
    def __lt__(self, other) -> bool:
        """Less than: this range ends before other starts (non-overlapping)"""
        if not isinstance(other, TimeRange):
            return NotImplemented

        if self.overlaps(other):
            raise ValueError(
                "lt comparison operator only applies to non-overlapping intervals"
            )

        return self.end <= other.start

    @validate_timezone_compatibility
    def __le__(self, other) -> bool:
        """Less than or equal"""
        if not isinstance(other, TimeRange):
            return NotImplemented

        if self.overlaps(other):
            raise ValueError(
                "le comparison operator only applies to non-overlapping intervals"
            )

        return self < other or self == other

    def __add__(self, td) -> "TimeRange":
        """Adds td timedelta to start and end"""
        if not isinstance(td, timedelta):
            return NotImplemented

        return TimeRange(start=self.start + td, end=self.end + td)

    def __hash__(self) -> int:
        return hash((self.start, self.end))
