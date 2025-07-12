from elastisched.blob import Blob
from typing import List
"""
Schedule

A Schedule tracks the current datetime, timezone, as well as future and past events.
All past events are frozen, and hence cannot be rescheduled. Future events are "realized"
but can change when new blobs are added. 

An event is a blob that has been "realized". Only future blobs may be rescheduled. 

"""
