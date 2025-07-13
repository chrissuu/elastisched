from enum import Enum
from datetime import timedelta
from typing import List, Dict, Optional, Callable, Any, Tuple
from dataclasses import dataclass, field
from elastisched.constants import *


@dataclass
class Policy:
    """Defines how a blob can be scheduled"""

    is_splittable: bool = False
    is_overlappable: bool = False
    is_invisible: bool = False
    max_splits: Optional[int] = None
    min_split_duration: timedelta = field(
        default_factory=lambda: MINIMUM_BLOB_SPLIT_DURATION
    )

    # policies we want:
    # can overlap
    # can split
    # invisible
    def __post_init__(self):
        if self.is_splittable:
            if self.max_splits is None:
                raise ValueError("Splittable policy needs max_splits to be defined.")
