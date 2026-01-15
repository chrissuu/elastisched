from datetime import datetime

from core.blob import Blob
from core.recurrence import MultipleBlobOccurrence
from core.timerange import TimeRange


def _make_blob(start: datetime, end: datetime) -> Blob:
    default_tr = TimeRange(start=start, end=end)
    schedulable_tr = TimeRange(start=start, end=end)
    return Blob(
        default_scheduled_timerange=default_tr,
        schedulable_timerange=schedulable_tr,
    )


def test_multiple_next_occurrence_picks_earliest_future():
    blob1 = _make_blob(
        datetime(2024, 1, 1, 10, 0),
        datetime(2024, 1, 1, 11, 0),
    )
    blob2 = _make_blob(
        datetime(2024, 1, 2, 10, 0),
        datetime(2024, 1, 2, 11, 0),
    )
    blob3 = _make_blob(
        datetime(2024, 1, 3, 10, 0),
        datetime(2024, 1, 3, 11, 0),
    )

    recurrence = MultipleBlobOccurrence(blobs=[blob3, blob1, blob2])
    next_occurrence = recurrence.next_occurrence(datetime(2023, 12, 31, 9, 0))

    assert next_occurrence is not None
    assert next_occurrence.get_schedulable_timerange().start == blob1.get_schedulable_timerange().start


def test_multiple_next_occurrence_skips_past_occurrences():
    blob1 = _make_blob(
        datetime(2024, 1, 1, 10, 0),
        datetime(2024, 1, 1, 11, 0),
    )
    blob2 = _make_blob(
        datetime(2024, 1, 2, 10, 0),
        datetime(2024, 1, 2, 11, 0),
    )

    recurrence = MultipleBlobOccurrence(blobs=[blob1, blob2])
    next_occurrence = recurrence.next_occurrence(datetime(2024, 1, 1, 12, 0))

    assert next_occurrence is not None
    assert next_occurrence.get_schedulable_timerange().start == blob2.get_schedulable_timerange().start


def test_multiple_all_occurrences_in_range():
    blob1 = _make_blob(
        datetime(2024, 1, 1, 10, 0),
        datetime(2024, 1, 1, 11, 0),
    )
    blob2 = _make_blob(
        datetime(2024, 1, 2, 10, 0),
        datetime(2024, 1, 2, 11, 0),
    )
    blob3 = _make_blob(
        datetime(2024, 1, 5, 10, 0),
        datetime(2024, 1, 5, 11, 0),
    )

    recurrence = MultipleBlobOccurrence(blobs=[blob1, blob2, blob3])
    search_range = TimeRange(
        start=datetime(2024, 1, 2, 0, 0),
        end=datetime(2024, 1, 4, 0, 0),
    )
    occurrences = recurrence.all_occurrences(search_range)

    assert len(occurrences) == 1
    assert occurrences[0].get_schedulable_timerange().start == blob2.get_schedulable_timerange().start


def test_multiple_all_occurrences_includes_overlap():
    blob1 = _make_blob(
        datetime(2024, 1, 1, 23, 0),
        datetime(2024, 1, 2, 2, 0),
    )
    blob2 = _make_blob(
        datetime(2024, 1, 3, 10, 0),
        datetime(2024, 1, 3, 11, 0),
    )
    recurrence = MultipleBlobOccurrence(blobs=[blob1, blob2])
    search_range = TimeRange(
        start=datetime(2024, 1, 2, 0, 0),
        end=datetime(2024, 1, 2, 1, 0),
    )

    occurrences = recurrence.all_occurrences(search_range)

    assert len(occurrences) == 1
    assert occurrences[0].get_schedulable_timerange().start == blob1.get_schedulable_timerange().start
