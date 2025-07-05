from datetime import datetime, timedelta
from daytime import daytime
from abc import ABC, abstractmethod
from types import List, Optional
from dataclasses import dataclass, field
from blob import Blob
from utils import TimeRange

def has_overlapping_blobs(L: List[Blob]) -> bool:
    sorted_blobs = sorted(L, key=lambda b: b.valid_schedulable_timerange.start)

    for i in range(len(sorted_blobs) - 1):
        if sorted_blobs[i].overlaps(sorted_blobs[i+1]):
            return False
        
    return True

class BlobRecurrence(ABC):
    """Abstract base class for recurrence rules""" 
    @abstractmethod
    def next_occurrence(self, current: datetime) -> Optional[Blob]:
        """Generate the next occurrence after the given datetime"""
        pass
    
    @abstractmethod
    def all_occurrences(self, timerange: TimeRange) -> List[Blob]:
        """Generate all occurrences within the given time range"""
        pass

class SingleBlobOccurrence(BlobRecurrence):
    blob: Blob = field(default_factory=Blob())

    def next_occurrence(self, current: datetime) -> Optional[Blob]:
        timerange = self.blob.get_timerange()
        
        if current < timerange.start
            return self.blob
        
        return None
        
    def all_occurrences(self, timerange: TimeRange) -> List[Blob]:
        if timerange.contains(self.blob.get_timerange()):
            return [self.blob]

        return []

@dataclass
class WeeklyBlobRecurrence(BlobRecurrence):
    """Weekly recurrence rule"""
    blobs_of_week: List[Blob]  # 0=Monday, 6=Sunday
    interval: int = 1  # Every N weeks
    __days_of_week: Optional[List[daytime]] = field(default=None)
    
    def __post_init__(self):
        if has_overlapping_blobs(self.blobs_of_week):
            raise ValueError("Weekly blob recurrence requires non-overlapping blobs")
        self.blobs_of_week.sort()
        self.__days_of_week = [
            daytime(blob.get_timerange().start.weekday(), blob.get_timerange().start.time())
            for blob in self.blobs_of_week
        ]
    
    def next_occurrence(self, current: datetime) -> Optional[datetime]:
        curr_daytime = daytime(current.weekday(), current.time())
        
        for day in self.days_of_week:
            if day > curr_daytime:
                days_ahead = day.day_of_week - current.weekday()
                next_date = current.date() + timedelta(days=days_ahead)
                return datetime.combine(next_date, day.time)
        
        days_to_next_week = (7 * self.interval) - current.weekday()
        next_week_start = current.date() + timedelta(days=days_to_next_week)
        
        if self.days_of_week:
            first_day = self.days_of_week[0]
            days_to_add = first_day.day_of_week
            next_date = next_week_start + timedelta(days=days_to_add)
            return datetime.combine(next_date, first_day.time)
        
        return None
    
    def all_occurrences(self, start: datetime, end: datetime) -> List[datetime]:
        occurrences = []
        
        current = start
        
        while True:
            next_occ = self.next_occurrence(current)
            if next_occ is None or next_occ > end:
                break
            
            occurrences.append(next_occ)
            current = next_occ + timedelta(seconds=1)
        
        return occurrences

@dataclass
class DeltaBlobRecurrence(BlobRecurrence):
    """Delta recurrence rule - recurring events at fixed time intervals"""
    delta: timedelta
    start_datetime: datetime
    
    def next_occurrence(self, current: datetime) -> Optional[datetime]:
        if current < self.start_datetime:
            return self.start_datetime
        
        time_diff = current - self.start_datetime
        intervals_passed = time_diff // self.delta
        
        next_occurrence = self.start_datetime + (intervals_passed + 1) * self.delta
        
        return next_occurrence
    
    def all_occurrences(self, start: datetime, end: datetime) -> List[datetime]:
        occurrences = []
        
        if start <= self.start_datetime:
            curr_datetime = self.start_datetime
        else:
            time_diff = start - self.start_datetime
            intervals_passed = time_diff // self.delta
            curr_datetime = self.start_datetime + intervals_passed * self.delta
            
            if curr_datetime < start:
                curr_datetime += self.delta
        
        while curr_datetime <= end:
            occurrences.append(curr_datetime)
            curr_datetime += self.delta
        
        return occurrences

@dataclass
class DateBlobRecurrence(BlobRecurrence):
    date: datetime
    
    def next_occurrence(self, current: datetime) -> Optional[datetime]:
        if self.date.month == 2 and self.date.day == 29:
            return self._next_leap_day(current)
        
        try:
            target_this_year = datetime(
                year=current.year,
                month=self.date.month,
                day=self.date.day,
                hour=self.date.hour,
                minute=self.date.minute,
                second=self.date.second,
                microsecond=self.date.microsecond
            )
            
            if target_this_year > current:
                return target_this_year
        except ValueError:
            # This shouldn't happen for non-leap days, but handle it just in case
            pass
        
        target_next_year = datetime(
            year=current.year + 1,
            month=self.date.month,
            day=self.date.day,
            hour=self.date.hour,
            minute=self.date.minute,
            second=self.date.second,
            microsecond=self.date.microsecond
        )
        return target_next_year
    
    def _next_leap_day(self, current: datetime) -> datetime:
        """Find the next Feb 29 after the current datetime"""
        def is_leap_year(year):
            return year % 4 == 0 and (year % 100 != 0 or year % 400 == 0)
        
        # Check if current year is a leap year and Feb 29 hasn't passed yet
        if is_leap_year(current.year):
            feb29_this_year = datetime(
                year=current.year,
                month=2,
                day=29,
                hour=self.date.hour,
                minute=self.date.minute,
                second=self.date.second,
                microsecond=self.date.microsecond
            )
            if feb29_this_year > current:
                return feb29_this_year
        
        # Find the next leap year
        next_year = current.year + 1
        while not is_leap_year(next_year):
            next_year += 1
        
        return datetime(
            year=next_year,
            month=2,
            day=29,
            hour=self.date.hour,
            minute=self.date.minute,
            second=self.date.second,
            microsecond=self.date.microsecond
        )
    
    def all_occurrences(self, start: datetime, end: datetime) -> List[datetime]:
        occurrences = []
        
        # Special handling for leap day (Feb 29)
        if self.date.month == 2 and self.date.day == 29:
            return self._all_leap_day_occurrences(start, end)
        
        # Start from the year of the start date
        current_year = start.year
        
        while True:
            # Create the target date for this year
            target_date = datetime(
                year=current_year,
                month=self.date.month,
                day=self.date.day,
                hour=self.date.hour,
                minute=self.date.minute,
                second=self.date.second,
                microsecond=self.date.microsecond
            )
            
            # Check if this occurrence is within our range
            if target_date > end:
                break
            
            if target_date >= start:
                occurrences.append(target_date)
            
            current_year += 1
        
        return occurrences
    
    def _all_leap_day_occurrences(self, start: datetime, end: datetime) -> List[datetime]:
        """Get all Feb 29 occurrences between start and end datetimes"""
        def is_leap_year(year):
            return year % 4 == 0 and (year % 100 != 0 or year % 400 == 0)
        
        occurrences = []
        current_year = start.year
        
        while current_year <= end.year:
            if is_leap_year(current_year):
                target_date = datetime(
                    year=current_year,
                    month=2,
                    day=29,
                    hour=self.date.hour,
                    minute=self.date.minute,
                    second=self.date.second,
                    microsecond=self.date.microsecond
                )
                
                if start <= target_date <= end:
                    occurrences.append(target_date)
            
            current_year += 1
        
        return occurrences