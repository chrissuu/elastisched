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
    policy = engine.Policy(0, 0, 0)
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
    split_policy = engine.Policy(1, HOUR, 1, True)
    rigid_policy = engine.Policy(0, 0, 0)

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
