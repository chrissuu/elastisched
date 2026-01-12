from .constants import *
import engine
import math


"""
DEPRECATED COST FUNCTIONS TESTS

Note: currently the cost function only implements cost function primitives:

split cost
illegal schedule cost
overlap cost

Everything else is handled by preference learner
"""
# def test_thursday_no_cost():
#     # Given
#     tag = engine.Tag(WORK_TAG)
#     policy = engine.Policy(0, 0, 0)
#     tr_schedulable = engine.TimeRange(Day.THURSDAY * DAY + Hour.TWELVE_AM * HOUR, 
#                                     Day.SATURDAY * DAY + Hour.ELEVEN_PM * HOUR)
#     tr_scheduled = engine.TimeRange(Day.THURSDAY * DAY + Hour.ELEVEN_PM * HOUR, 
#                                     Day.THURSDAY * DAY + Hour.ELEVEN_PM * HOUR + 30 * MINUTE)
#     duration = tr_scheduled.getHigh() - tr_scheduled.getLow()

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
#         duration = tr_scheduled.getHigh() - tr_scheduled.getLow()

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
#     duration = tr_scheduled.getHigh() - tr_scheduled.getLow()

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
#     duration = tr_scheduled.getHigh() - tr_scheduled.getLow()

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
#     duration = tr_scheduled.getHigh() - tr_scheduled.getLow()

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
