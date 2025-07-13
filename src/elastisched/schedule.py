from datetime import datetime, timedelta, timezone
from dataclasses import dataclass, field
from elastisched.blob import Blob
from elastisched.constants import DEFAULT_TZ
from elastisched.timerange import TimeRange
from elastisched.engine import schedule
from typing import List, Tuple, Dict
"""
Schedule

A Schedule tracks the current datetime, timezone, as well as future and past events.
All past events are frozen, and hence cannot be rescheduled. Future events are "realized"
but can change when new blobs are added. 

An event is a blob that has been "realized". Only future blobs may be rescheduled. 

"""
class Schedule:
    id_blob_map: Dict[str, Blob]
    realized_blobs: List[Tuple[str, TimeRange]] # List[(blob_id, realized_blob_range)]
    tz: timezone = field(default_factory=lambda: DEFAULT_TZ)

    def schedule(self, id_blob_map: Dict[str, Blob]) -> bool:
        pass

    def add_blob(self, blob: Blob) -> bool:
        """
        Adds a blob to the schedule and calls the scheduler API. 
        Returns True if the blob was successfully scheduled, and False otherwise.
        """

        if blob.id in self.id_blob_map.keys():
            return False
        
        self.id_blob_map[blob.id] = blob
        
        was_successful = self.schedule(self.id_blob_map)

        return was_successful
    
    def remove_blob(self, id: str) -> bool:
        """
        Removes a blob by id from the schedule and calls the scheduler API. 
        Returns True if the blob was successfully removed, and False otherwise.
        """

        if id not in self.id_blob_map.keys():
            raise ValueError("Id must be present in id_blob_map")
        
        del self.id_blob_map[id]

        was_successful = self.schedule(self.id_blob_map)

        return was_successful
    

        








        
        


    


