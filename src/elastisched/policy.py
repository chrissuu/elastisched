from dataclasses import dataclass, field
from datetime import timedelta

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
