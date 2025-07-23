from datetime import datetime, timedelta, timezone
from dataclasses import dataclass, field
from elastisched.blob import Blob
from elastisched.constants import DEFAULT_TZ
from elastisched.timerange import TimeRange
from engine import schedule
from typing import List, Tuple, Dict

"""
Schedule

A Schedule is a state-less class holding realized blobs. State-less means that it has
no concept of a "now"/current datetime. Realized blobs are blobs which have been scheduled
by the scheduler API.

"""


class Schedule:
    id_blob_map: Dict[str, Blob]
    realized_blobs: List[Tuple[str, TimeRange]]  # List[(blob_id, realized_blob_range)]
    tz: timezone = field(default_factory=lambda: DEFAULT_TZ)

    def schedule(self) -> bool:
        pass

    def add_blob(self, blob: Blob) -> bool:
        """
        Adds a blob to the schedule and calls the scheduler API.
        Returns True if the blob was successfully scheduled, and False otherwise.
        """

        if blob.id in self.id_blob_map.keys():
            return False

        self.id_blob_map[blob.id] = blob

        successful = self.schedule()

        return successful

    def remove_blob(self, id: str) -> bool:
        """
        Removes a blob by id from the schedule and calls the scheduler API.
        Returns True if the blob was successfully removed, and False otherwise.
        """

        if id not in self.id_blob_map.keys():
            raise ValueError("Id must be present in id_blob_map")

        successful = self.schedule(self.id_blob_map)

        if successful:
            del self.id_blob_map[id]

        return successful

    def get_events_in_range(self, timerange: TimeRange) -> List[str]:
        events = []
        for realized_blob in self.realized_blobs:
            (id, tr) = realized_blob
            if timerange.contains(tr):
                events.append(id)

        return events
