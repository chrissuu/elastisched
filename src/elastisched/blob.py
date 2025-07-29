import uuid
from dataclasses import dataclass, field
from datetime import datetime, timezone
from typing import List, Optional, Set

from elastisched.timerange import TimeRange
from elastisched.constants import *
from engine import Policy, Tag, Job, TimeRange as tr


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

    def to_job(self, EPOCH_BEGIN: datetime) -> Job:
        schedulable_timerange_start = self.schedulable_timerange.start
        schedulable_timerange_end = self.schedulable_timerange.end
        schedulable_tr = tr(
            schedulable_timerange_start - EPOCH_BEGIN,
            schedulable_timerange_end - EPOCH_BEGIN,
        )

        scheduled_timerange_start = self.default_scheduled_timerange.start
        scheduled_timerange_end = self.default_scheduled_timerange.end
        scheduled_tr = tr(
            scheduled_timerange_start - EPOCH_BEGIN,
            scheduled_timerange_end - EPOCH_BEGIN,
        )

        return Job(
            int(self.duration.total_seconds()),
            schedulable_tr,
            scheduled_tr,
            self._id,
            self.policy,
            self.dependencies,
            self.tags,
        )
    
    @classmethod
    def from_job(cls, 
        job: Job, 
        EPOCH_BEGIN: datetime, 
        name: str,
        description: str,
        tz: timezone
    ) -> 'Blob':
        scheduled_tr_start_delta = job.schedulableTimeRange.getLow()
        scheduled_tr_end_delta = job.schedulableTimeRange.getHigh()
        scheduled_timerange=  TimeRange(
            start=EPOCH_BEGIN + timedelta(seconds=scheduled_tr_start_delta),
            end=EPOCH_BEGIN + timedelta(seconds=scheduled_tr_end_delta)
        )

        schedulable_tr_start_delta = job.schedulableTimeRange.getLow()
        schedulable_tr_end_delta = job.schedulableTimeRange.getHigh()
        schedulable_timerange = TimeRange(
            start=EPOCH_BEGIN + timedelta(seconds=schedulable_tr_start_delta),
            end=EPOCH_BEGIN + timedelta(seconds=schedulable_tr_end_delta)
        )

        return Blob(
            default_scheduled_timerange=scheduled_timerange,
            schedulable_timerange=schedulable_timerange,
            name=name,
            description=description,
            tz=tz,
            policy=job.policy,
            dependencies=job.dependencies,
            id=job.id,
            tags=job.tags
        )

    def __str__(self) -> str:
        """String representation"""
        return ""

    def __eq__(self, other) -> bool:
        """Check equality with another daytime object"""
        if not isinstance(other, Blob):
            return False
        return self.id == other.get_id()

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

    def get_id(self) -> str:
        return self.id

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
