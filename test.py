import engine
from elastisched.blob import Blob
from datetime import datetime
from datetime import timezone

def main():
    # Create a Tag
    tag = engine.Tag("work")
    print("Tag name:", tag.name)

    # Create a Policy
    policy = engine.Policy(0, 0, 0)
    print("Policy is splittable?", policy.isSplittable())

    # Create a TimeRange
    tr_schedulable = engine.TimeRange(0, 60 * 60)  # 0 to 1 hour
    tr_scheduled = engine.TimeRange(0, 60 * 30)  # 0 to 30 min

    # Create a Job
    job = engine.Job(
        60 * 30,  # duration: 30 min
        tr_schedulable,
        tr_scheduled,
        "job1",
        policy,
        set(),  # dependencies
        {tag},  # tags
    )

    # Create a Schedule and add the job
    sched = engine.Schedule()
    sched.addJob(job)
    print("Number of jobs in schedule:", len(sched.scheduledJobs))

    # Run the engine.function
    jobs = [job]
    result = engine.schedule_jobs(
        jobs, 60 * 15, 1000.0, 1.0, 100000000
    )  # granularity: 15 min, start_epoch: 0
    for e in result.scheduledJobs:
        print(e)

    blob = Blob.from_job(result.scheduledJobs[0], datetime.now(), "TestBlob", "Hello world!", timezone.utc)
    print(f"Blob id: {blob.get_id()}")
    print(f"Blob policy: {blob.policy.isSplittable()}")


if __name__ == "__main__":
    main()
