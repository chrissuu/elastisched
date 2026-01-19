from .constants import *
import engine
import pytest


def _make_job(
    schedulable_low,
    schedulable_high,
    scheduled_low,
    scheduled_high,
    policy=None,
    job_id="job",
):
    policy = policy or engine.Policy(0, 0, 0)
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


"""
DEPRECATED COST FUNCTIONS TESTS

Note: currently the cost function only implements cost function primitives:

split cost
illegal schedule cost
overlap cost

Everything else is handled by preference learner
"""


def test_illegal_schedule_cost_outside_schedulable_range():
    job = _make_job(
        schedulable_low=0,
        schedulable_high=HOUR,
        scheduled_low=2 * HOUR,
        scheduled_high=3 * HOUR,
    )
    schedule = engine.Schedule([job])
    cost_function = engine.ScheduleCostFunction(schedule, GRANULARITY)

    assert cost_function.schedule_cost() == pytest.approx(1e12, rel=1e-6)


def test_overlap_cost_counts_overlap_duration():
    overlappable_policy = engine.Policy(0, 0, 2)
    job_a = _make_job(
        schedulable_low=0,
        schedulable_high=4 * HOUR,
        scheduled_low=0,
        scheduled_high=HOUR,
        policy=overlappable_policy,
        job_id="job_a",
    )
    job_b = _make_job(
        schedulable_low=0,
        schedulable_high=4 * HOUR,
        scheduled_low=30 * MINUTE,
        scheduled_high=90 * MINUTE,
        policy=overlappable_policy,
        job_id="job_b",
    )
    schedule = engine.Schedule([job_a, job_b])
    cost_function = engine.ScheduleCostFunction(schedule, MINUTE)

    assert cost_function.schedule_cost() == 30.0


def test_split_cost_counts_number_of_splits():
    job = _make_job(
        schedulable_low=0,
        schedulable_high=6 * HOUR,
        scheduled_low=HOUR,
        scheduled_high=2 * HOUR,
        job_id="split_job",
    )
    job.scheduled_time_ranges = [
        engine.TimeRange(HOUR, 2 * HOUR),
        engine.TimeRange(3 * HOUR, 4 * HOUR),
    ]
    schedule = engine.Schedule([job])
    cost_function = engine.ScheduleCostFunction(schedule, MINUTE)

    assert cost_function.schedule_cost() == 10.0
# def test_thursday_no_cost():
#     # Given
#     tag = engine.Tag(WORK_TAG)
#     policy = engine.Policy(0, 0, 0)
#     tr_schedulable = engine.TimeRange(Day.THURSDAY * DAY + Hour.TWELVE_AM * HOUR, 
#                                     Day.SATURDAY * DAY + Hour.ELEVEN_PM * HOUR)
#     tr_scheduled = engine.TimeRange(Day.THURSDAY * DAY + Hour.ELEVEN_PM * HOUR, 
#                                     Day.THURSDAY * DAY + Hour.ELEVEN_PM * HOUR + 30 * MINUTE)
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
    
#     schedule = engine.Schedule([job])

#     cost_function = engine.ScheduleCostFunction(schedule, GRANULARITY)

#     # Assert that
#     assert(cost_function.schedule_cost() < EPSILON)

# def test_busy_day_constant_cost():
#     # Given
#     for day in range(5):
#         tag = engine.Tag(WORK_TAG)
#         policy = engine.Policy(0, 0, 0)
#         tr_schedulable = engine.TimeRange(day * DAY + Hour.TWELVE_AM * HOUR, 
#                                         (day + 1) * DAY + Hour.ELEVEN_PM * HOUR)
#         tr_scheduled = engine.TimeRange(day * DAY + Hour.ELEVEN_PM * HOUR, 
#                                         day * DAY + Hour.ELEVEN_PM * HOUR + 30 * MINUTE)
#         duration = tr_scheduled.get_high() - tr_scheduled.get_low()

#         job = engine.Job(
#             duration,
#             tr_schedulable,
#             tr_scheduled,
#             "test_job",
#             policy,
#             set(),  # dependencies
#             {tag},  # tags
#         )

#         schedule = engine.Schedule([job])

#         cost_function = engine.ScheduleCostFunction(schedule, GRANULARITY)

#         # Assert that
#         assert(abs(cost_function.busy_day_constant_cost(day) - 0.5) < EPSILON)


# def test_next_saturday_cost():
#     # Given
#     tag = engine.Tag(WORK_TAG)
#     policy = engine.Policy(0, 0, 0)
#     tr_schedulable = engine.TimeRange(WEEK + Day.SATURDAY * DAY + Hour.TWELVE_AM * HOUR, 
#                                       WEEK + Day.SUNDAY * DAY + Hour.ELEVEN_PM * HOUR)
#     tr_scheduled = engine.TimeRange(WEEK + Day.SATURDAY * DAY + Hour.ELEVEN_PM * HOUR, 
#                                     WEEK + Day.SATURDAY * DAY + Hour.ELEVEN_PM * HOUR + 30 * MINUTE)
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

#     schedule = engine.Schedule([job])

#     cost_function = engine.ScheduleCostFunction(schedule, GRANULARITY)

#     # Assert that
#     assert(abs(cost_function.schedule_cost() - 0.5) < EPSILON)

# def test_busy_afternoon_cost():
#     # Given
#     day = Day.MONDAY

#     tag = engine.Tag(WORK_TAG)
#     policy = engine.Policy(0, 0, 0)
#     tr_schedulable = engine.TimeRange(day * DAY + Hour.TWELVE_AM * HOUR, 
#                                     (day + 1) * DAY + Hour.ELEVEN_PM * HOUR)
#     tr_scheduled = engine.TimeRange(day * DAY + Hour.ELEVEN_PM * HOUR, 
#                                     day * DAY + Hour.ELEVEN_PM * HOUR + 30 * MINUTE)
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

#     schedule = engine.Schedule([job])

#     cost_function = engine.ScheduleCostFunction(schedule, GRANULARITY)
#     print(cost_function.busy_afternoon_exponential_cost(day.value))

#     # Assert that
#     assert(abs(cost_function.busy_afternoon_exponential_cost(day.value) - math.exp(EXP_DOWNFACTOR * tr_scheduled.length() / HOUR)) < EPSILON)

# def test_fri_cost():
#     # Given
#     tag = engine.Tag(WORK_TAG)
#     policy = engine.Policy(0, 0, 0)
#     tr_schedulable = engine.TimeRange(Day.THURSDAY * DAY + Hour.TWELVE_AM * HOUR, 
#                                     Day.SATURDAY * DAY + Hour.ELEVEN_PM * HOUR)
#     tr_scheduled = engine.TimeRange(Day.FRIDAY * DAY + 1 * MINUTE, 
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

#     schedule = engine.Schedule([job])

#     cost_function = engine.ScheduleCostFunction(schedule, GRANULARITY)
#     print(cost_function.busy_afternoon_exponential_cost(Day.FRIDAY.value))

#     assert(cost_function.schedule_cost() < EPSILON)
