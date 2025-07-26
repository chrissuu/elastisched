from dataclasses import dataclass, field
from datetime import timedelta
from typing import Optional
from elastisched.constants import *


@dataclass
class Policy:
    """Defines how a blob can be scheduled"""
    is_splittable: bool = False
    is_overlappable: bool = False
    is_invisible: bool = False
    max_splits: int = 0
    min_split_duration: timedelta = field(
        default_factory=lambda: MINIMUM_BLOB_SPLIT_DURATION
    )
    
    def __post_init__(self):
        if self.is_splittable:
            if self.max_splits is None:
                raise ValueError("Splittable policy needs max_splits to be defined.")
    
    def to_dict(self):
        return {
            'is_splittable': self.is_splittable,
            'is_overlappable': self.is_overlappable,
            'is_invisible': self.is_invisible,
            'max_splits': self.max_splits,
            'min_split_duration_seconds': self.min_split_duration.total_seconds()
        }
    
    @classmethod
    def from_dict(cls, data):
        min_split_duration = timedelta(seconds=data.get('min_split_duration_seconds', 
                                                       MINIMUM_BLOB_SPLIT_DURATION.total_seconds()))
        
        return cls(
            is_splittable=data.get('is_splittable', False),
            is_overlappable=data.get('is_overlappable', False),
            is_invisible=data.get('is_invisible', False),
            max_splits=data.get('max_splits', 0),
            min_split_duration=min_split_duration
        )
    
    def __str__(self):
        flags = []
        if self.is_splittable:
            flags.append(f"splittable(max:{self.max_splits})")
        if self.is_overlappable:
            flags.append("overlappable")
        if self.is_invisible:
            flags.append("invisible")
        
        if flags:
            return f"Policy({', '.join(flags)})"
        return "Policy(default)"