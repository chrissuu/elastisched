import engine
from .constants import *
from .constants import RANDOM_TEST_ITERATIONS


def test_fri_sat_cost():
    for i in range(RANDOM_TEST_ITERATIONS):
        # Given
        tag = engine.Tag(WORK_TAG)
        policy = engine.Policy(0, 0, 0)
        tr_schedulable = engine.TimeRange(Day.THURSDAY * DAY + Hour.TWELVE_AM * HOUR, 
                                        Day.SATURDAY * DAY + Hour.ELEVEN_PM * HOUR)
        tr_scheduled = engine.TimeRange(Day.FRIDAY * DAY, Day.FRIDAY * DAY + 30 * MINUTE)
        duration = tr_scheduled.getHigh() - tr_scheduled.getLow()

        job = engine.Job(
            duration,
            tr_schedulable,
            tr_scheduled,
            "test_job",
            policy,
            set(),  # dependencies
            {tag},  # tags
        )

        schedule = engine.schedule([job], GRANULARITY)

        # Assert that
        for job in schedule.scheduledJobs:
            curr_scheduled_tr = job.scheduledTimeRange
            assert((curr_scheduled_tr.getLow() // DAY) == Day.THURSDAY)


def test_friday_exponential_cost():
    for i in range(RANDOM_TEST_ITERATIONS):
        # Given
        tag = engine.Tag(WORK_TAG)
        policy = engine.Policy(0, 0, 0)
        tr_schedulable = engine.TimeRange(Day.FRIDAY * DAY, 
                                        Day.FRIDAY * DAY + Hour.ELEVEN_PM * HOUR)
        tr_scheduled = engine.TimeRange(Day.FRIDAY * DAY + Hour.SEVEN_PM * HOUR, Day.FRIDAY * DAY + Hour.SEVEN_PM * HOUR + 30 * MINUTE)
        duration = tr_scheduled.getHigh() - tr_scheduled.getLow()

        job = engine.Job(
            duration,
            tr_schedulable,
            tr_scheduled,
            "test_job",
            policy,
            set(),  # dependencies
            {tag},  # tags
        )

        schedule = engine.schedule([job], GRANULARITY)

        # Assert that
        for job in schedule.scheduledJobs:
            curr_scheduled_tr = job.scheduledTimeRange

            # Friday jobs are exponentially discounted
            # Singleton friday jobs should be scheduled before the
            # afternoon cutoff time
            assert(not ((curr_scheduled_tr.getLow() - Day.FRIDAY * DAY) // HOUR) > AFTERNOON_START)


def test_scheduler_invariance_cost():
    # Given
    tag = engine.Tag(WORK_TAG)
    policy = engine.Policy(0, 0, 0)
    tr_schedulable = engine.TimeRange(Day.FRIDAY * DAY, 
                                      Day.FRIDAY * DAY + Hour.ELEVEN_PM * HOUR)
    tr_scheduled = engine.TimeRange(Day.FRIDAY * DAY + Hour.FOUR_PM * HOUR, Day.FRIDAY * DAY + Hour.FOUR_PM * HOUR + 30 * MINUTE)
    duration = tr_scheduled.getHigh() - tr_scheduled.getLow()

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

    job = schedule.scheduledJobs[0]
    curr_scheduled_tr = job.scheduledTimeRange
    assert(curr_scheduled_tr.getLow() == tr_scheduled.getLow())
