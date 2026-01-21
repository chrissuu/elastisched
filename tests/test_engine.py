import engine
from .constants import *
from .constants import RANDOM_TEST_ITERATIONS
import pytest

# @pytest.mark.repeat(RANDOM_TEST_ITERATIONS)
# def test_fri_sat_cost():
#     # Given
#     tag = engine.Tag(WORK_TAG)
#     policy = engine.Policy(0, 0, 0)
#     tr_schedulable = engine.TimeRange(Day.THURSDAY * DAY + Hour.TWELVE_AM * HOUR, 
#                                     Day.SATURDAY * DAY + Hour.ELEVEN_PM * HOUR)
#     tr_scheduled = engine.TimeRange(Day.FRIDAY * DAY, 
#                                     Day.FRIDAY * DAY + 30 * MINUTE)
#     duration = tr_scheduled.get_high() - tr_scheduled.get_low()

#     job = engine.Job(
#         duration,
#         tr_schedulable,
#         tr_scheduled,
#         "test_job",
#         policy,
#         set(),  # dependencies
#         {tag},  # tags
#     )

#     schedule, cost_history = engine.schedule_jobs([job], GRANULARITY, 1000.0, 1.0, 10000000000)
#     print(cost_history)

#     # Assert that
#     for job in schedule.scheduled_jobs:
#         curr_scheduled_tr = job.scheduled_time_range
#         curr_schedulable_tr = job.schedulable_time_range
#         assert(curr_scheduled_tr.get_low() <= Day.FRIDAY * DAY + AFTERNOON_START * HOUR)
#         assert(curr_schedulable_tr.contains(curr_scheduled_tr))


# @pytest.mark.repeat(RANDOM_TEST_ITERATIONS)
# def test_friday_exponential_cost():
#     # Given
#     tag = engine.Tag(WORK_TAG)
#     policy = engine.Policy(0, 0, 0)
#     tr_schedulable = engine.TimeRange(Day.FRIDAY * DAY, 
#                                       Day.FRIDAY * DAY + Hour.ELEVEN_PM * HOUR)
#     tr_scheduled = engine.TimeRange(Day.FRIDAY * DAY + Hour.SEVEN_PM * HOUR, 
#                                     Day.FRIDAY * DAY + Hour.SEVEN_PM * HOUR + 30 * MINUTE)
#     duration = tr_scheduled.get_high() - tr_scheduled.get_low()


#     job = engine.Job(
#         duration,
#         tr_schedulable,
#         tr_scheduled,
#         "test_job",
#         policy,
#         set(),  # dependencies
#         {tag},  # tags
#     )

#     schedule, cost_history = engine.schedule_jobs([job], GRANULARITY, 1000.0, 1.0, 100000)
#     print(cost_history)

#     print(Day.FRIDAY * DAY + Hour.SEVEN_PM * HOUR)
#     # Assert that
#     for job in schedule.scheduled_jobs:
#         curr_scheduled_tr = job.scheduled_time_range
#         curr_schedulable_tr = job.schedulable_time_range

#         # Friday jobs are exponentially discounted
#         # Singleton friday jobs should be scheduled before the
#         # afternoon cutoff time
#         assert(not ((curr_scheduled_tr.get_low() // HOUR) % 24) > AFTERNOON_START)
#         assert(curr_schedulable_tr.contains(curr_scheduled_tr))


@pytest.mark.repeat(RANDOM_TEST_ITERATIONS)
def test_scheduler_invariance_cost():
    # Given
    tag = engine.Tag(WORK_TAG)
    policy = engine.Policy(0, 0)
    tr_schedulable = engine.TimeRange(Day.FRIDAY * DAY, 
                                      Day.FRIDAY * DAY + Hour.ELEVEN_PM * HOUR)
    tr_scheduled = engine.TimeRange(Day.FRIDAY * DAY + Hour.FOUR_PM * HOUR, 
                                    Day.FRIDAY * DAY + Hour.FOUR_PM * HOUR + 30 * MINUTE)
    duration = tr_scheduled.get_high() - tr_scheduled.get_low()

    job = engine.Job(
        duration,
        tr_schedulable,
        tr_scheduled,
        "test_job",
        policy,
        set(),  # dependencies
        {tag},  # tags
    )

    schedule, cost_history = engine.schedule_jobs([job], GRANULARITY, 1000.0, 1.0, 100000000)
    print(cost_history)

    job = schedule.scheduled_jobs[0]
    curr_scheduled_tr = job.scheduled_time_range
    curr_schedulable_tr = job.schedulable_time_range
    assert(curr_schedulable_tr.contains(curr_scheduled_tr))
    assert(curr_scheduled_tr.get_low() == tr_scheduled.get_low())


def test_force_split_schedule(monkeypatch):
    monkeypatch.setenv("ELASTISCHED_RNG_SEED", "4242")
    split_policy = engine.Policy(1, HOUR, True, False, False, True)
    rigid_policy = engine.Policy(0, 0)

    schedulable = engine.TimeRange(Day.MONDAY * DAY + Hour.NINE_AM * HOUR,
                                   Day.MONDAY * DAY + Hour.TWELVE_PM * HOUR)
    rigid_range = engine.TimeRange(Day.MONDAY * DAY + Hour.TEN_AM * HOUR,
                                   Day.MONDAY * DAY + Hour.ELEVEN_AM * HOUR)

    split_job = engine.Job(
        2 * HOUR,
        schedulable,
        engine.TimeRange(Day.MONDAY * DAY + Hour.NINE_AM * HOUR,
                         Day.MONDAY * DAY + Hour.ELEVEN_AM * HOUR),
        "split_job",
        split_policy,
        set(),
        set(),
    )

    rigid_job = engine.Job(
        HOUR,
        rigid_range,
        rigid_range,
        "rigid_job",
        rigid_policy,
        set(),
        set(),
    )

    schedule, _ = engine.schedule_jobs([split_job, rigid_job], HOUR, 1000.0, 0.01, 50000)
    scheduled_split = next(job for job in schedule.scheduled_jobs if job.id == "split_job")
    ranges = list(scheduled_split.scheduled_time_ranges)

    assert len(ranges) == 2
    total_duration = sum(r.get_high() - r.get_low() for r in ranges)
    assert total_duration == split_job.duration
    assert all((r.get_high() - r.get_low()) >= HOUR for r in ranges)
    assert all(r.get_low() % HOUR == 0 for r in ranges)


def _make_job(
    schedulable_low,
    schedulable_high,
    scheduled_low,
    scheduled_high,
    policy=None,
    job_id="job",
):
    policy = policy or engine.Policy(0, 0)
    tr_schedulable = engine.TimeRange(schedulable_low, schedulable_high)
    tr_scheduled = engine.TimeRange(scheduled_low, scheduled_high)
    duration = tr_scheduled.get_high() - tr_scheduled.get_low()
    return engine.Job(
        duration,
        tr_schedulable,
        tr_scheduled,
        job_id,
        policy,
        set(),
        set(),
    )


def test_timerange_overlap_contains_length():
    range_a = engine.TimeRange(0, 2 * HOUR)
    range_b = engine.TimeRange(HOUR, 3 * HOUR)
    range_c = engine.TimeRange(2 * HOUR, 3 * HOUR)
    range_inside = engine.TimeRange(30 * MINUTE, HOUR)

    assert range_a.overlaps(range_b)
    assert not range_a.overlaps(range_c)
    assert range_a.contains(range_inside)
    assert range_a.length() == 2 * HOUR


def test_tag_equality_and_hashing():
    tag_a = engine.Tag("work", "primary")
    tag_b = engine.Tag("work", "secondary")
    tag_c = engine.Tag("rest", "secondary")

    assert tag_a == tag_b
    assert tag_a != tag_c
    assert len({tag_a, tag_b, tag_c}) == 2


def test_schedule_jobs_preserves_rigid_times():
    rigid_schedulable = engine.TimeRange(Day.MONDAY * DAY + Hour.NINE_AM * HOUR,
                                         Day.MONDAY * DAY + Hour.TEN_AM * HOUR)
    rigid_scheduled = engine.TimeRange(Day.MONDAY * DAY + Hour.TEN_AM * HOUR,
                                       Day.MONDAY * DAY + Hour.ELEVEN_AM * HOUR)
    rigid_job = engine.Job(
        HOUR,
        rigid_schedulable,
        rigid_scheduled,
        "rigid_job",
        engine.Policy(0, 0),
        set(),
        set(),
    )

    schedule, _ = engine.schedule_jobs([rigid_job], HOUR, 10.0, 0.1, 500)
    scheduled_job = schedule.scheduled_jobs[0]

    assert scheduled_job.scheduled_time_range == rigid_schedulable
    assert scheduled_job.scheduled_time_ranges[0] == rigid_schedulable
