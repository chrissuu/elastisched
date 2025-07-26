import uuid
from dataclasses import dataclass, field
from datetime import datetime, timezone
from typing import List, Optional, Set

from elastisched.constants import *
from elastisched.policy import Policy
from elastisched.tag import Tag
from elastisched.timerange import TimeRange


@dataclass
class Blob:
    """Core scheduling unit representing a task/event"""

    default_scheduled_timerange: TimeRange
    schedulable_timerange: TimeRange
    name: str = field(default="Unnamed Blob")
    description: Optional[str] = field(default=None)
    tz: timezone = field(default_factory=lambda: DEFAULT_TZ)
    policy: Policy = field(default_factory=lambda: Policy)

    dependencies: List[str] = field(default_factory=list)  # IDs of other blobs
    id: str = field(default_factory=lambda: str(uuid.uuid4()))
    tags: Set[Tag] = field(default_factory=set)

    def __post_init__(self):
        self.duration: timedelta = self.default_scheduled_timerange.duration()
        if not self.schedulable_timerange.contains(self.default_scheduled_timerange):
            raise ValueError(
                "Valid schedulable range must contain default scheduled timerange"
            )

    def to_dict(self) -> dict:
        """
        Convert the Blob object to a dictionary representation
        Returns:
            dict: Dictionary containing all blob fields
        """
        return {
            'default_scheduled_timerange': self.default_scheduled_timerange.to_dict(),
            'schedulable_timerange': self.schedulable_timerange.to_dict(),
            'name': self.name,
            'description': self.description,
            'tz': str(self.tz),
            'policy': self.policy.to_dict(),
            'dependencies': self.dependencies.copy(),
            'id': self.id,
            'tags': [tag.to_dict() for tag in self.tags]
        }
    
    @classmethod
    def from_dict(cls, data: dict) -> 'Blob':
        """
        Create a Blob object from a dictionary representation
        Args:
            data: Dictionary containing blob fields
        Returns:
            Blob: New Blob object
        Raises:
            KeyError: If required keys are missing from the dictionary
            ValueError: If values are invalid
        """
        required_keys = ['default_scheduled_timerange', 'schedulable_timerange']
        missing_keys = [key for key in required_keys if key not in data]
        if missing_keys:
            raise KeyError(f"Missing required keys: {missing_keys}")
        
        try:
            tz_str = data.get('tz')
            if tz_str:
                if tz_str == 'UTC':
                    tz = timezone.utc
                else:
                    tz = DEFAULT_TZ  # Fallback to default
            else:
                tz = DEFAULT_TZ
            
            tags_data = data.get('tags', [])
            tags = {Tag.from_dict(tag_dict) for tag_dict in tags_data}
            
            return cls(
                default_scheduled_timerange=TimeRange.from_dict(data['default_scheduled_timerange']),
                schedulable_timerange=TimeRange.from_dict(data['schedulable_timerange']),
                name=data.get('name', "Unnamed Blob"),
                description=data.get('description'),
                tz=tz,
                policy=Policy.from_dict(data.get('policy', {})),
                dependencies=data.get('dependencies', []).copy(),
                id=data.get('id', str(uuid.uuid4())),
                tags=tags
            )
        except (ValueError, TypeError, AttributeError) as e:
            raise ValueError(f"Invalid data format: {e}")
        
    def __str__(self) -> str:
        """String representation"""
        return ""

    def __eq__(self, other) -> bool:
        """Check equality with another daytime object"""
        if not isinstance(other, Blob):
            return False
        return self.id == other.id

    def __lt__(self, other) -> bool:
        if not isinstance(other, Blob):
            raise NotImplemented

        return self.schedulable_timerange < other.schedulable_timerange

    def __le__(self, other) -> bool:
        if not isinstance(other, Blob):
            raise NotImplemented

        return self.schedulable_timerange <= other.schedulable_timerange

    def __hash__(self) -> int:
        return hash(self.id)

    def get_duration(self) -> timedelta:
        return self.duration

    def get_default_scheduled_timerange(self) -> TimeRange:
        return self.default_scheduled_timerange

    def get_schedulable_timerange(self) -> TimeRange:
        return self.schedulable_timerange

    def get_policy(self) -> Policy:
        return self.policy

    def set_default_scheduled_timerange(self, timerange: TimeRange):
        self.default_scheduled_timerange = timerange
        return

    def set_schedulable_timerange(self, timerange: TimeRange):
        self.schedulable_timerange = timerange
        return

    def overlaps(self, other) -> bool:
        return self.schedulable_timerange.overlaps(other.schedulable_timerange)