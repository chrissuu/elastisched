from enum import Enum
from datetime import timedelta
from typing import List, Dict, Optional, Callable, Any, Tuple
from dataclasses import dataclass

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
    
    def can_overlap(self) -> bool:
        return self.policy_type == PolicyType.RIGID
    
    def is_fixed(self) -> bool:
        return self.policy_type == PolicyType.FLEXIBLE
    
    def is_invisible(self) -> bool:
        return self.policy_type == PolicyType.INVISIBLE