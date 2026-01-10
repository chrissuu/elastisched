from abc import ABC, abstractmethod
from copy import deepcopy
from dataclasses import dataclass, field
from datetime import datetime, timedelta
from typing import List, Optional
import uuid

from elastisched.blob import Blob
from elastisched.daytime import daytime
from engine import TimeRange


def has_overlapping_blobs(blobs: List[Blob]) -> bool:
    sorted_blobs = sorted(blobs, key=lambda b: b.schedulable_timerange.start)

    for i in range(len(sorted_blobs) - 1):
        if sorted_blobs[i].overlaps(sorted_blobs[i + 1]):
            return True

    return False


def _coerce_datetime(value: datetime, tzinfo) -> datetime:
    if tzinfo is None:
        return value
    if value.tzinfo is None:
        return value.replace(tzinfo=tzinfo)
    return value.astimezone(tzinfo)


def blob_copy_with_delta_future(blob: Blob, td: timedelta):
    blob_copy = deepcopy(blob)

    curr_default_scheduled_timerange = blob_copy.get_default_scheduled_timerange()
    blob_copy.set_default_scheduled_timerange(curr_default_scheduled_timerange + td)

    curr_schedulable_timerange = blob_copy.get_schedulable_timerange()
    blob_copy.set_schedulable_timerange(curr_schedulable_timerange + td)

    return blob_copy


@dataclass
class BlobRecurrence(ABC):
    """Abstract base class for recurrence rules"""

    _id: str = field(default_factory=lambda: str(uuid.uuid4()), init=False)

    @abstractmethod
    def next_occurrence(self, current: datetime) -> Optional[Blob]:
        """Generate the next occurrence after the given datetime"""
        pass

    @abstractmethod
    def all_occurrences(self, timerange: TimeRange) -> List[Blob]:
        """Generate all occurrences within the given time range"""
        pass

    def __eq__(self, other):
        if not isinstance(other, BlobRecurrence):
            return False
        return self._id == other.get_id()

    def get_id(self):
        return self._id

    def __hash__(self):
        return hash(self._id)


@dataclass
class SingleBlobOccurrence(BlobRecurrence):
    blob: Blob

    def next_occurrence(self, current: datetime) -> Optional[Blob]:
        timerange = self.blob.get_schedulable_timerange()

        current_local = _coerce_datetime(current, timerange.start.tzinfo)
        if current_local < timerange.start:
            return self.blob

        return None

    def all_occurrences(self, timerange: TimeRange) -> List[Blob]:
        if timerange.contains(self.blob.get_schedulable_timerange()):
            return [deepcopy(self.blob)]

        return []


@dataclass
class WeeklyBlobRecurrence(BlobRecurrence):
    """Weekly recurrence rule"""

    blobs_of_week: List[Blob]
    interval: int = 1  # Every N weeks

    def __post_init__(self):
        if len(self.blobs_of_week) == 0:
            raise ValueError(
                "There must be at least one occurrence in a weekly recurrence"
            )

        if has_overlapping_blobs(self.blobs_of_week):
            raise ValueError("Weekly blob recurrence requires non-overlapping blobs")

        self.blobs_of_week.sort()
        self.__days_of_week = []
        for blob in self.blobs_of_week:
            base_start = blob.get_schedulable_timerange().start
            tzinfo = blob.tz or base_start.tzinfo
            base_start_local = base_start.astimezone(tzinfo) if tzinfo else base_start
            self.__days_of_week.append(
                daytime(base_start_local.weekday(), base_start_local.time())
            )

    def next_occurrence(self, current: datetime) -> Optional[Blob]:
        occurrences = []

        for blob, day in zip(self.blobs_of_week, self.__days_of_week):
            base_start = blob.get_schedulable_timerange().start
            tzinfo = blob.tz or base_start.tzinfo
            current_local = current
            base_start_local = base_start
            if tzinfo:
                current_local = _coerce_datetime(current, tzinfo)
                base_start_local = base_start.astimezone(tzinfo)

            total_days = (current_local - base_start_local).days
            weeks_since_start = max(0, total_days // 7)  # clamp to 0 if before start
            interval_start = (weeks_since_start // self.interval) * self.interval

            candidate_weeks = [
                interval_start,
                interval_start + self.interval,
            ]

            for week_offset in candidate_weeks:
                days_since_base = week_offset * 7 + (
                    day.day_of_week - base_start_local.weekday()
                )
                occurrence_date = base_start_local + timedelta(days=days_since_base)
                occurrence_datetime = datetime.combine(
                    occurrence_date.date(), day.time, tzinfo=tzinfo
                )

                if occurrence_datetime < base_start_local:
                    continue

                if occurrence_datetime > current_local:
                    occurrence_project = (
                        occurrence_datetime.astimezone(base_start.tzinfo)
                        if base_start.tzinfo
                        else occurrence_datetime
                    )
                    delta = occurrence_project - base_start
                    future_blob = blob_copy_with_delta_future(blob, delta)
                    occurrences.append((occurrence_datetime, future_blob))
                    break

        if not occurrences:
            return None

        occurrences.sort(key=lambda tup: tup[0])
        return occurrences[0][1]

    def all_occurrences(self, timerange: TimeRange) -> List[Blob]:
        occurrences = []
        current = timerange.start

        while True:
            next_blob = self.next_occurrence(current)
            print(next_blob.get_schedulable_timerange().start)
            if next_blob is None:
                break

            if not timerange.contains(next_blob.get_schedulable_timerange()):
                break

            next_start = next_blob.get_schedulable_timerange().start
            if next_start > timerange.end:
                break

            occurrences.append(next_blob)
            current = next_start + timedelta(microseconds=1)

        return occurrences


@dataclass
class DeltaBlobRecurrence(BlobRecurrence):
    """Delta recurrence rule - recurring events at fixed time intervals"""

    delta: timedelta
    start_blob: Blob

    def __post_init__(self):
        schedulable_timerange = self.start_blob.get_schedulable_timerange()
        if schedulable_timerange.duration() > self.delta:
            raise ValueError(
                "Blob schedulable timerange duration should not be larger than delta"
            )

    def next_occurrence(self, current: datetime) -> Optional[Blob]:
        start = self.start_blob.get_schedulable_timerange().start
        current_local = _coerce_datetime(current, start.tzinfo)
        if current_local < start:
            return deepcopy(self.start_blob)

        time_diff = current_local - start
        intervals_passed = time_diff // self.delta
        delta_to_occurrence = (intervals_passed + 1) * self.delta

        return blob_copy_with_delta_future(self.start_blob, delta_to_occurrence)

    def all_occurrences(self, timerange: TimeRange) -> List[Blob]:
        occurrences = []

        start_schedulable_timerange = self.start_blob.get_schedulable_timerange()
        start = timerange.start
        end = timerange.end

        # Determine the first occurrence to consider
        if start <= start_schedulable_timerange.start:
            curr_datetime = start_schedulable_timerange.start
        else:
            time_diff = start - start_schedulable_timerange.start
            intervals_passed = time_diff // self.delta
            curr_datetime = (
                start_schedulable_timerange.start + intervals_passed * self.delta
            )

            if curr_datetime < start:
                curr_datetime += self.delta

        while curr_datetime <= end:
            time_diff = curr_datetime - start_schedulable_timerange.start
            intervals_passed = time_diff // self.delta
            delta_to_occurrence = intervals_passed * self.delta

            blob_copy = blob_copy_with_delta_future(
                self.start_blob, delta_to_occurrence
            )

            if timerange.contains(blob_copy.get_schedulable_timerange()):
                occurrences.append(blob_copy)
            else:
                break

            curr_datetime += self.delta

        return occurrences


@dataclass
class DateBlobRecurrence(BlobRecurrence):
    blob: Blob

    def __post_init__(self):
        timerange = self.blob.get_schedulable_timerange()
        start: datetime = timerange.start
        end: datetime = timerange.end
        tzinfo = self.blob.tz or start.tzinfo
        start_local = start.astimezone(tzinfo) if tzinfo else start
        end_local = end.astimezone(tzinfo) if tzinfo else end

        if start_local.weekday() != end_local.weekday():
            raise ValueError(
                "Date blob recurrence should have a blob with timerange that starts and ends on the same day"
            )

        if start_local.year != end_local.year:
            raise ValueError(
                "Date blob recurrence should have a blob with timerange that starts and ends on the same year"
            )

    def next_occurrence(self, current: datetime) -> Optional[Blob]:
        schedulable_timerange = self.blob.get_schedulable_timerange()
        start = schedulable_timerange.start
        tzinfo = self.blob.tz or start.tzinfo
        dt: datetime = schedulable_timerange.start
        start_local = start.astimezone(tzinfo) if tzinfo else start
        current_local = _coerce_datetime(current, tzinfo)
        dt_local = dt.astimezone(tzinfo) if tzinfo else dt
        date = dt_local.date()
        time = dt_local.time()

        if start_local.date().month == 2 and start_local.date().day == 29:
            return self._next_leap_day(current_local)

        try:
            target_this_year = datetime(
                year=current_local.year,
                month=date.month,
                day=date.day,
                hour=time.hour,
                minute=time.minute,
                second=time.second,
                microsecond=time.microsecond,
                tzinfo=tzinfo,
            )

            if target_this_year > current_local:
                if current_local.year >= start_local.year:
                    years_diff = current_local.year - start_local.year
                    actual_target = datetime(
                        year=start_local.year + years_diff,
                        month=date.month,
                        day=date.day,
                        hour=time.hour,
                        minute=time.minute,
                        second=time.second,
                        microsecond=time.microsecond,
                        tzinfo=tzinfo,
                    )
                    actual_target_project = (
                        actual_target.astimezone(start.tzinfo)
                        if start.tzinfo
                        else actual_target
                    )
                    delta_to_occurrence = actual_target_project - start
                    return blob_copy_with_delta_future(self.blob, delta_to_occurrence)
                else:
                    return self.blob

        except ValueError:
            # This shouldn't happen for non-leap days, but handle it just in case
            pass

        if current_local.year >= start_local.year:
            years_diff = (current_local.year + 1) - start_local.year
        else:
            return self.blob

        target_next_year = datetime(
            year=start_local.year + years_diff,
            month=date.month,
            day=date.day,
            hour=time.hour,
            minute=time.minute,
            second=time.second,
            microsecond=time.microsecond,
            tzinfo=tzinfo,
        )

        target_next_project = (
            target_next_year.astimezone(start.tzinfo) if start.tzinfo else target_next_year
        )
        delta_to_occurrence = target_next_project - start
        return blob_copy_with_delta_future(self.blob, delta_to_occurrence)

    def _next_leap_day(self, current: datetime) -> Blob:
        """Find the next Feb 29 after the current datetime"""
        schedulable_timerange = self.blob.get_schedulable_timerange()
        start = schedulable_timerange.start
        tzinfo = self.blob.tz or start.tzinfo
        start_local = start.astimezone(tzinfo) if tzinfo else start
        dt = schedulable_timerange.start
        dt_local = dt.astimezone(tzinfo) if tzinfo else dt
        date = dt_local.date()
        time = dt_local.time()

        def is_leap_year(year):
            return year % 4 == 0 and (year % 100 != 0 or year % 400 == 0)

        # Check if current year is a leap year and Feb 29 hasn't passed yet
        if is_leap_year(current.year):
            feb29_this_year = datetime(
                year=current.year,
                month=2,
                day=29,
                hour=time.hour,
                minute=time.minute,
                second=time.second,
                microsecond=time.microsecond,
                tzinfo=tzinfo,
            )
            if feb29_this_year > current:
                if current.year >= start_local.year:
                    # Current year is same or after the original blob's year
                    years_diff = current.year - start_local.year
                    actual_target = datetime(
                        year=start_local.year + years_diff,
                        month=2,
                        day=29,
                        hour=time.hour,
                        minute=time.minute,
                        second=time.second,
                        microsecond=time.microsecond,
                        tzinfo=tzinfo,
                    )
                    actual_target_project = (
                        actual_target.astimezone(start.tzinfo)
                        if start.tzinfo
                        else actual_target
                    )
                    delta_to_occurrence = actual_target_project - start
                    return blob_copy_with_delta_future(self.blob, delta_to_occurrence)
                else:
                    # Current year is before the original blob's year, return original blob
                    return self.blob

        # Find the next leap year
        next_year = current.year + 1
        while not is_leap_year(next_year):
            next_year += 1

        if next_year >= start_local.year:
            years_diff = next_year - start_local.year
        else:
            # If next leap year is before original blob's year, return original blob
            return self.blob

        feb29_next = datetime(
            year=start_local.year + years_diff,
            month=2,
            day=29,
            hour=time.hour,
            minute=time.minute,
            second=time.second,
            microsecond=time.microsecond,
            tzinfo=tzinfo,
        )
        feb29_next_project = (
            feb29_next.astimezone(start.tzinfo) if start.tzinfo else feb29_next
        )
        delta_to_occurrence = feb29_next_project - start
        return blob_copy_with_delta_future(self.blob, delta_to_occurrence)

    def all_occurrences(self, timerange: TimeRange) -> List[Blob]:
        occurrences = []
        start = timerange.start
        end = timerange.end
        schedulable_timerange = self.blob.get_schedulable_timerange()
        schedulable_timerange_start = schedulable_timerange.start
        tzinfo = self.blob.tz or schedulable_timerange_start.tzinfo

        dt = schedulable_timerange_start
        dt_local = dt.astimezone(tzinfo) if tzinfo else dt
        date = dt_local.date()
        time = dt_local.time()

        if date.month == 2 and date.day == 29:
            return self._all_leap_day_occurrences(timerange)

        start_local = start.astimezone(tzinfo) if tzinfo else start
        end_local = end.astimezone(tzinfo) if tzinfo else end
        current_year = start_local.year

        while True:
            try:
                target_date = datetime(
                    year=current_year,
                    month=date.month,
                    day=date.day,
                    hour=time.hour,
                    minute=time.minute,
                    second=time.second,
                    microsecond=time.microsecond,
                    tzinfo=tzinfo,
                )
            except ValueError:
                current_year += 1
                continue

            target_project = (
                target_date.astimezone(schedulable_timerange_start.tzinfo)
                if schedulable_timerange_start.tzinfo
                else target_date
            )
            delta_to_occurrence = target_project - schedulable_timerange_start
            blob_copy = blob_copy_with_delta_future(self.blob, delta_to_occurrence)

            # Check if this occurrence is within our range
            if target_date > end_local or (
                not timerange.contains(blob_copy.get_schedulable_timerange())
            ):
                break

            if target_date >= start_local:
                occurrences.append(blob_copy)

            current_year += 1

        return occurrences

    def _all_leap_day_occurrences(self, timerange: TimeRange) -> List[Blob]:
        """Get all Feb 29 occurrences between start and end datetimes"""

        def is_leap_year(year):
            return year % 4 == 0 and (year % 100 != 0 or year % 400 == 0)

        occurrences = []
        start = timerange.start
        end = timerange.end
        schedulable_timerange = self.blob.get_schedulable_timerange()
        tzinfo = self.blob.tz or schedulable_timerange.start.tzinfo
        start_local = start.astimezone(tzinfo) if tzinfo else start
        end_local = end.astimezone(tzinfo) if tzinfo else end

        dt = schedulable_timerange.start
        dt_local = dt.astimezone(tzinfo) if tzinfo else dt
        date = dt_local.date()
        time = dt_local.time()

        current_year = start_local.year

        while current_year <= end_local.year:
            if is_leap_year(current_year):
                target_date = datetime(
                    year=current_year,
                    month=2,
                    day=29,
                    hour=time.hour,
                    minute=time.minute,
                    second=time.second,
                    microsecond=time.microsecond,
                    tzinfo=tzinfo,
                )

                target_project = (
                    target_date.astimezone(schedulable_timerange.start.tzinfo)
                    if schedulable_timerange.start.tzinfo
                    else target_date
                )
                delta_to_occurrence = target_project - schedulable_timerange.start

                blob_copy = blob_copy_with_delta_future(self.blob, delta_to_occurrence)

                if start_local <= target_date <= end_local and timerange.contains(
                    blob_copy.get_schedulable_timerange()
                ):
                    occurrences.append(blob_copy)

            current_year += 1

        return occurrences
