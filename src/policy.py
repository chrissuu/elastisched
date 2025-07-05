from enum import Enum
from datetime import timedelta
from typing import List, Dict, Optional, Callable, Any, Tuple
from dataclasses import dataclass

from constants import *

class PolicyType(Enum):
    RIGID = "rigid"
    FLEXIBLE = "flexible"
    INVISIBLE = "invisible" 

@dataclass
class Policy:
    """Defines how a blob can be scheduled"""
    policy_type: PolicyType
    is_splittable: bool = False
    max_splits: Optional[int] = None
    min_split_duration: Optional[int] = timedelta(minutes=MINIMUM_BLOB_SPLIT_DURATION)

    def __post_init__(self):
        if self.is_splittable:
            if self.max_splits is None:
                raise ValueError("Splittable policy needs max_splits to be defined.")
    
    def is_fixed(self) -> bool:
        return self.policy_type == PolicyType.RIGID

    def can_overlap(self) -> bool:
        return self.policy_type == PolicyType.FLEXIBLE
    
    def is_invisible(self) -> bool:
        return self.policy_type == PolicyType.INVISIBLE