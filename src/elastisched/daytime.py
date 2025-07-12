from datetime import time 
from typing import Union

class daytime:
    """Represents a specific day of the week with a time"""
    
    def __init__(self, day_of_week: int, time: Union[time, str]):
        """
        Initialize a daytime object
        
        Args:
            day_of_week: Integer representing day (0=Monday, 6=Sunday)
            time: Either a time object or string in format "HH:MM" or "HH:MM:SS"
        """
        if not isinstance(day_of_week, int) or day_of_week < 0 or day_of_week > 6:
            raise ValueError("day_of_week must be an integer between 0 (Monday) and 6 (Sunday)")
        
        self.day_of_week = day_of_week
        
        if isinstance(time, str):
            self.time = self._parse_time_string(time)
        elif isinstance(time, time):
            self.time = time
        else:
            raise ValueError("time must be a time object or string in format 'HH:MM' or 'HH:MM:SS'")
    
    def _parse_time_string(self, time_str: str) -> time:
        """Parse time string into time object"""
        try:
            # Try HH:MM:SS format first
            if time_str.count(':') == 2:
                hour, minute, second = map(int, time_str.split(':'))
                return time(hour, minute, second)
            # Try HH:MM format
            elif time_str.count(':') == 1:
                hour, minute = map(int, time_str.split(':'))
                return time(hour, minute)
            else:
                raise ValueError("Invalid time format")
        except ValueError:
            raise ValueError("time string must be in format 'HH:MM' or 'HH:MM:SS'")
    
    def __str__(self) -> str:
        """String representation"""
        days = ['Monday', 'Tuesday', 'Wednesday', 'Thursday', 'Friday', 'Saturday', 'Sunday']
        return f"{days[self.day_of_week]} at {self.time.strftime('%H:%M:%S')}"
    
    def __repr__(self) -> str:
        """Developer-friendly representation"""
        return f"daytime(day_of_week={self.day_of_week}, time='{self.time}')"
    
    def __eq__(self, other) -> bool:
        """Check equality with another daytime object"""
        if not isinstance(other, daytime):
            return False
        return self.day_of_week == other.day_of_week and self.time == other.time
    
    def __lt__(self, other) -> bool:
        """Less than comparison for sorting"""
        if not isinstance(other, daytime):
            raise TypeError("Cannot compare daytime with non-Daytime object")
        
        if self.day_of_week != other.day_of_week:
            return self.day_of_week < other.day_of_week
        return self.time < other.time
    
    def __hash__(self) -> int:
        """Make hashable so it can be used in sets and as dict keys"""
        return hash((self.day_of_week, self.time))
    
    def weekday(self):
        return self.day_of_week
    
    @property
    def day_name(self) -> str:
        """Get the name of the day"""
        days = ['Monday', 'Tuesday', 'Wednesday', 'Thursday', 'Friday', 'Saturday', 'Sunday']
        return days[self.day_of_week]
    
    @property
    def is_weekend(self) -> bool:
        """Check if this is a weekend day"""
        return self.day_of_week in [5, 6]  # Saturday, Sunday
    
    @property
    def is_weekday(self) -> bool:
        """Check if this is a weekday"""
        return not self.is_weekend