from datetime import datetime, timedelta, timezone
from dataclasses import dataclass, field
from elastisched.blob import Blob
from elastisched.constants import DEFAULT_TZ
from elastisched.timerange import TimeRange
from typing import List, Tuple, Dict
"""
Schedule

A Schedule tracks the current datetime, timezone, as well as future and past events.
All past events are frozen, and hence cannot be rescheduled. Future events are "realized"
but can change when new blobs are added. 

An event is a blob that has been "realized". Only future blobs may be rescheduled. 

"""
@dataclass
class Schedule:
    id_blobs_map: Dict[str, List[Blob]]
    realized_blobs: List[Tuple[str, TimeRange]] # List[(blob_id, realized_blob_range)]
    tz: timezone = field(default_factory=lambda: DEFAULT_TZ)

    def add_blob(self, blob: Blob) -> bool:
        """
        Adds a blob to the schedule and calls the scheduler API. Returns True if the
        blob was successfully scheduled, and False otherwise.
        """

        if blob.id in self.id_blobs_map.keys():
            return False
        
        


    


