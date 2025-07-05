from datetime import datetime, timedelta, timezone
from utils import TimeRange

DEFAULT_TZ = timezone.utc
DEFAULT_END_DATE = datetime(year=datetime.MAXYEAR, month=12, day=31)

DEFAULT_BLOB_DURATION = timedelta(minutes=30)
DEFAULT_BLOB_SCHEDULED_AFTER_NOW = timedelta(minutes=60)
DEFAULT_BLOB_SCHEDULABLE_AFTER_NOW = timedelta(minutes=120)

MINIMUM_BLOB_SPLIT_DURATION = timedelta(minutes=15)

GRANULARITY = timedelta(minutes=5)

ENTIRE_TIME_RANGE = TimeRange(
    start=datetime(year=datetime.MINYEAR, month=1, day=1),
    end=DEFAULT_END_DATE
)