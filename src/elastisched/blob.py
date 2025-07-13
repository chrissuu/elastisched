import uuid
from datetime import datetime, timezone
from dataclasses import dataclass, field
from elastisched.constants import *
from elastisched.policy import Policy
from elastisched.recurrence import *
from elastisched.tag import Tag
from elastisched.timerange import TimeRange
from elastisched.utils import round_datetime_future_bias

@dataclass
class Blob:
    """Core scheduling unit representing a task/event"""

    default_scheduled_timerange: TimeRange
    schedulable_timerange: TimeRange
    name: str = field(default="Unnamed Blob")
    description: Optional[str] = field(default=None)
    tz: timezone = field(default_factory=lambda: DEFAULT_TZ)
    policy: Policy = field(default_factory=lambda: Policy)

    # Optional fields
    dependencies: List[str] = field(default_factory=list)  # IDs of other blobs
    id: str = field(default_factory=lambda: str(uuid.uuid4()))
    tag: List[Tag] = field(default_factory=List)

    # Scheduling metadata
    __actual_scheduled_timerange: Optional[TimeRange] = field(default=None)
    __actual_duration: Optional[timedelta] = field(default=None)
    __completed: bool = field(default=False)
    __created_at: datetime = field(default=datetime.now())

    def __post_init__(self):
        if not self.schedulable_timerange.contains(self.default_scheduled_timerange):
            raise ValueError(
                "Valid schedulable range must contain [default_datetime, default_datetime+duration]"
            )

    def __str__(self) -> str:
        """String representation"""
        return ""

    def __repr__(self) -> str:
        """Developer-friendly representation"""
        return f"daytime(day_of_week={self.day_of_week}, time='{self.time}')"

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
        """Make hashable so it can be used in sets and as dict keys"""
        return hash(self.id)

    def get_duration(self) -> Optional[timedelta]:
        if self.__actual_scheduled_timerange is not None:
            return self.__actual_scheduled_timerange.duration()
        return None

    def get_default_scheduled_timerange(self) -> TimeRange:
        return self.default_scheduled_timerange

    def get_schedulable_timerange(self) -> TimeRange:
        return self.schedulable_timerange

    def set_default_scheduled_timerange(self, timerange: TimeRange):
        self.default_scheduled_timerange = timerange
        return

    def set_scheduble_timerange(self, timerange: TimeRange):
        self.schedulable_timerange = timerange
        return

    def set_completed(self, completed):
        self.__completed = completed
        return

    def overlaps(self, other) -> bool:
        return self.schedulable_timerange.overlaps(other.schedulable_timerange)
