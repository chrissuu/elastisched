from datetime import datetime, timedelta, timezone, date

DEFAULT_TZ = timezone.utc

DEFAULT_START_DATE = datetime(
    date.min.year, date.min.month, date.min.day, tzinfo=timezone.utc
)
DEFAULT_END_DATE = datetime(
    date.max.year, date.max.month, date.max.day, tzinfo=timezone.utc
)
MIN_DATE = date.min
MAX_DATE = date.max

DEFAULT_BLOB_DURATION = timedelta(minutes=30)
DEFAULT_BLOB_SCHEDULED_AFTER_NOW = timedelta(minutes=60)
DEFAULT_BLOB_SCHEDULABLE_AFTER_NOW = timedelta(minutes=120)

MINIMUM_BLOB_SPLIT_DURATION = timedelta(minutes=15)

GRANULARITY = timedelta(minutes=5)
