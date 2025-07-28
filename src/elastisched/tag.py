from dataclasses import dataclass
from typing import Optional

@dataclass
class Tag:
    name: str
    group: Optional[str] = None
    
    def __str__(self):
        if self.group:
            return f"{self.group}:{self.name}"
        return self.name
    
    def __hash__(self):
        # Include group in hash so tags with same name but different groups are different
        return hash((self.name, self.group))
    
    def __eq__(self, other):
        if isinstance(other, Tag):
            return self.name == other.name and self.group == other.group
        return False