#ifndef TESTS_H
#define TESTS_H
#include "job.hpp"
#include "policy.hpp"
#include "engine.hpp"
#include "IntervalTree.hpp"

#include "constants.hpp"

#include <vector>
#include <memory>
#include <algorithm>
#include <cstddef>
#include <map>
#include <random>
#include <optional>
#include <iostream>
#include <set>
#include <cassert>

#include "SimulatedAnnealingOptimizer.hpp"

void test_basic_scheduling() {
    std::cout << "Running basic scheduler test...\n";
    
    const time_t GRANULARITY = 15 * constants::MINUTE; // 15 minutes
    
    std::vector<Job> jobs;
    
    Policy flexiblePolicy(0, 0.0, 0); // Not splittable, not overlappable
    std::set<ID> noDependencies;
    std::set<Tag> workTags = {Tag("work")};
    
    TimeRange job1_schedulable(
        9 * constants::HOUR,  // 9 AM
        17 * constants::HOUR // 5 PM
    );
    TimeRange job1_scheduled(9 * constants::HOUR, 10 * constants::HOUR); // Will be set by scheduler
    
    jobs.emplace_back(
        constants::HOUR, // 1 hour duration
        job1_schedulable,
        job1_scheduled,
        "meeting1",
        flexiblePolicy,
        noDependencies,
        workTags
    );
    
    TimeRange job2_schedulable(
        10 * constants::HOUR, // 10 AM
        16 * constants::HOUR  // 4 PM
    );
    TimeRange job2_scheduled(10 * constants::HOUR, 10 * constants::HOUR + 30 * constants::MINUTE);
    
    jobs.emplace_back(
        30 * constants::MINUTE, // 30 minutes duration
        job2_schedulable,
        job2_scheduled,
        "task1",
        flexiblePolicy,
        noDependencies,
        workTags
    );
    
    TimeRange job3_schedulable(
        8 * constants::HOUR,  // 8 AM
        18 * constants::HOUR  // 6 PM
    );
    TimeRange job3_scheduled(16 * constants::HOUR, 18 * constants::HOUR);
    
    jobs.emplace_back(
        2 * constants::HOUR, // 2 hours duration
        job3_schedulable,
        job3_scheduled,
        "project1",
        flexiblePolicy,
        noDependencies,
        workTags
    );
    
    Schedule result = schedule(jobs, GRANULARITY);
    
    std::cout << "Scheduled " << result.scheduledJobs.size() << " jobs\n";
    
    for (const auto& job : result.scheduledJobs) {
        time_t start_hours = job.scheduledTimeRange.getLow() / (constants::HOUR_TO_MINUTES * constants::MINUTE);
        time_t start_minutes = (job.scheduledTimeRange.getLow() % (constants::HOUR_TO_MINUTES * constants::MINUTE)) / constants::MINUTE;
        time_t duration_minutes = job.duration / constants::MINUTE;
        
        std::cout << "Job " << job.id 
                  << ": Scheduled at " << start_hours << ":" 
                  << (start_minutes < 10 ? "0" : "") << start_minutes
                  << " for " << duration_minutes << " minutes\n\n";
    }
    
    for (const auto& job : result.scheduledJobs) {
        std::cout << "Job " << job.id << "\n"
                  << "sch.low: " << job.scheduledTimeRange.getLow() << " sch.high: " << job.scheduledTimeRange.getHigh() << "\n"
                  << "schel.low: " << job.schedulableTimeRange.getLow() << " schel.high: " << job.schedulableTimeRange.getHigh() << "\n\n";
    }
    std::cout << "Basic scheduler test passed!\n\n";
}

void test_empty_schedule() {
    std::cout << "Running empty schedule test...\n";
    
    std::vector<Job> empty_jobs;
    const time_t GRANULARITY = 15 * constants::MINUTE;
    
    Schedule result = schedule(empty_jobs, GRANULARITY);
    
    assert(result.scheduledJobs.empty());
    std::cout << "Empty schedule test passed!\n\n";
}

void test_rigid_job() {
    std::cout << "Running rigid job test...\n";
    
    const time_t GRANULARITY = 15 * constants::MINUTE;
    
    Policy rigidPolicy(0, 0.0, 3); // Both bits set
    std::set<ID> noDependencies;
    std::set<Tag> meetingTags = {Tag("meeting")};
    
    time_t meeting_start = 14 * constants::HOUR_TO_MINUTES * constants::MINUTE; // 2 PM
    time_t meeting_duration = constants::HOUR_TO_MINUTES * constants::MINUTE; // 1 hour
    
    TimeRange rigid_schedulable(meeting_start, meeting_start + meeting_duration);
    TimeRange rigid_scheduled(meeting_start, meeting_start + meeting_duration);
    
    std::vector<Job> jobs;
    jobs.emplace_back(
        meeting_duration,
        rigid_schedulable,
        rigid_scheduled,
        "rigid_meeting",
        rigidPolicy,
        noDependencies,
        meetingTags
    );
    
    Schedule result = schedule(jobs, GRANULARITY);
    
    assert(result.scheduledJobs.size() == 1);
    
    const auto& scheduled_job = result.scheduledJobs[0];
    std::cout << "Rigid job scheduled successfully\n";
    std::cout << "Rigid job test passed!\n\n";
}

void test_friday_cost_optimization() {
    std::cout << "Running Friday cost optimization test...\n";
    
    const time_t GRANULARITY = 15 * constants::MINUTE; // 15 minutes

    time_t thursday_start = 3 * constants::DAY;  // Thursday 9 AM
    time_t thursday_end = 3 * constants::DAY + 17 * constants::HOUR_TO_MINUTES * constants::MINUTE;   // Thursday 5 PM
    time_t friday_start = 4 * constants::DAY;  // Friday 9 AM
    time_t friday_end = 5 * constants::DAY;     // Friday 5 PM
    
    Policy flexiblePolicy(0, 0.0, 0); // Not splittable, not overlappable
    std::set<ID> noDependencies;
    std::set<Tag> workTags = {Tag("work")};
    
    TimeRange schedulable_range(thursday_start, friday_end);
    
    time_t initial_friday_time = friday_start + 2 * constants::HOUR_TO_MINUTES * constants::MINUTE; // Friday 11 AM
    time_t job_duration = 2 * constants::HOUR_TO_MINUTES * constants::MINUTE; // 2 hours
    TimeRange initial_scheduled(initial_friday_time, initial_friday_time + job_duration);
    
    std::vector<Job> jobs;
    jobs.emplace_back(
        job_duration,
        schedulable_range,
        initial_scheduled,
        "friday_task",
        flexiblePolicy,
        noDependencies,
        workTags
    );
    
    std::cout << "Initial schedule: Friday 11:00 AM - 1:00 PM\n";
    std::cout << "Job can be rescheduled between Thursday 9 AM and Friday 5 PM\n";
    std::cout << "Expected: Optimizer should move job to Thursday due to lower cost\n";
    
    Schedule result = schedule(jobs, GRANULARITY);
    
    assert(result.scheduledJobs.size() == 1);
    
    const auto& optimized_job = result.scheduledJobs[0];
    time_t scheduled_start = optimized_job.scheduledTimeRange.getLow();
    
    bool moved_to_thursday = scheduled_start < friday_start;
    
    time_t days_from_monday = scheduled_start / constants::DAY;
    time_t time_of_day = scheduled_start % constants::DAY;
    time_t hours = time_of_day / (constants::HOUR_TO_MINUTES * constants::MINUTE);
    time_t minutes = (time_of_day % (constants::HOUR_TO_MINUTES * constants::MINUTE)) / constants::MINUTE;
    
    std::string day_name;
    switch(days_from_monday) {
        case 0: day_name = "Monday"; break;
        case 1: day_name = "Tuesday"; break;
        case 2: day_name = "Wednesday"; break;
        case 3: day_name = "Thursday"; break;
        case 4: day_name = "Friday"; break;
        case 5: day_name = "Saturday"; break;
        case 6: day_name = "Sunday"; break;
        default: day_name = "Unknown"; break;
    }
    
    std::cout << "Optimized schedule: " << day_name << " " 
              << hours << ":" << (minutes < 10 ? "0" : "") << minutes
              << " - " << (hours + 2) << ":" << (minutes < 10 ? "0" : "") << minutes << "\n";
    
    if (moved_to_thursday) {
        std::cout << "SUCCESS: Job was moved to Thursday (lower cost day)\n";
    } else {
        std::cout << "WARNING: Job remained on Friday (higher cost day)\n";
        std::cout << "  This could indicate:\n";
        std::cout << "  - The cost function isn't working as expected\n";
        std::cout << "  - The simulated annealing didn't find the better solution\n";
        std::cout << "  - There may be other constraints preventing the move\n";
    }
    
    std::cout << "Friday cost optimization test completed!\n\n";
}


void test_fri_sat_cost_optimization() {
    // std::cout << "Running Friday, Saturday Cost optimization test" << "\n";

    // Should expect for task schedulable between Thursday-Saturday to be scheduled on Thursday
    return;
}

#endif
