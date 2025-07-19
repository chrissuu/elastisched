import scheduler

def main():
    # Create a Tag
    tag = scheduler.Tag("work")
    print("Tag name:", tag.name)

    # Create a Policy
    policy = scheduler.Policy(0, 0.0, 0)
    print("Policy is splittable?", policy.isSplittable())

    # Create a TimeRange
    tr_schedulable = scheduler.TimeRange(0, 60*60)  # 0 to 1 hour
    tr_scheduled = scheduler.TimeRange(0, 60*30)    # 0 to 30 min

    # Create a Job
    job = scheduler.Job(
        60*30,  # duration: 30 min
        tr_schedulable,
        tr_scheduled,
        "job1",
        policy,
        set(),         # dependencies
        {tag}          # tags
    )

    # Create a Schedule and add the job
    sched = scheduler.Schedule()
    sched.addJob(job)
    print("Number of jobs in schedule:", len(sched.scheduledJobs))

    # Run the scheduler function
    jobs = [job]
    result = scheduler.schedule(jobs, len(jobs), 60*15, 0)  # granularity: 15 min, start_epoch: 0
    print("Scheduled jobs:", result.scheduledJobs)

if __name__ == "__main__":
    main()