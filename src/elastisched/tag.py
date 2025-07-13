from dataclasses import field, dataclass
from typing import Optional
from uuid import uuid4

@dataclass
class TagGroup:
    name: str
    id: str = field(default_factory=lambda: str(uuid4))

@dataclass
class Tag:
    name: str
    id: str = field(default_factory=lambda: str(uuid4()))
    group: Optional[TagGroup] = None
