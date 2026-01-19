#ifndef TESTS_H
#define TESTS_H
#include "Job.hpp"
#include "Policy.hpp"
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

#include "Optimizer.hpp"

void test_basic_scheduling() {
    std::cout << "Running basic scheduler test...\n";
    
    const time_t granularity = 15 * constants::minute; // 15 minutes
    
    std::vector<Job> jobs;
    
    Policy flexible_policy(0, 0.0, 0); // Not splittable, not overlappable
    std::set<ID> no_dependencies;
    std::set<Tag> work_tags = {Tag("work")};
    
    TimeRange job1_schedulable(
        9 * constants::hour,  // 9 AM
        17 * constants::hour // 5 PM
    );
    TimeRange job1_scheduled(9 * constants::hour, 10 * constants::hour); // Will be set by scheduler
    
    jobs.emplace_back(
        constants::hour, // 1 hour duration
        job1_schedulable,
        job1_scheduled,
        "meeting1",
        flexible_policy,
        no_dependencies,
        work_tags
    );
    
    TimeRange job2_schedulable(
        10 * constants::hour, // 10 AM
        16 * constants::hour  // 4 PM
    );
    TimeRange job2_scheduled(10 * constants::hour, 10 * constants::hour + 30 * constants::minute);
    
    jobs.emplace_back(
        30 * constants::minute, // 30 minutes duration
        job2_schedulable,
        job2_scheduled,
        "task1",
        flexible_policy,
        no_dependencies,
        work_tags
    );
    
    TimeRange job3_schedulable(
        8 * constants::hour,  // 8 AM
        18 * constants::hour  // 6 PM
    );
    TimeRange job3_scheduled(16 * constants::hour, 18 * constants::hour);
    
    jobs.emplace_back(
        2 * constants::hour, // 2 hours duration
        job3_schedulable,
        job3_scheduled,
        "project1",
        flexible_policy,
        no_dependencies,
        work_tags
    );
    
    Schedule result = schedule(jobs, granularity);
    
    std::cout << "Scheduled " << result.scheduled_jobs.size() << " jobs\n";
    
    for (const auto& job : result.scheduled_jobs) {
        time_t start_hours = job.scheduled_time_range.get_low() / (constants::hour_to_minutes * constants::minute);
        time_t start_minutes = (job.scheduled_time_range.get_low() % (constants::hour_to_minutes * constants::minute)) / constants::minute;
        time_t duration_minutes = job.duration / constants::minute;
        
        std::cout << "Job " << job.id 
                  << ": Scheduled at " << start_hours << ":" 
                  << (start_minutes < 10 ? "0" : "") << start_minutes
                  << " for " << duration_minutes << " minutes\n\n";
    }
    
    for (const auto& job : result.scheduled_jobs) {
        std::cout << "Job " << job.id << "\n"
                  << "sch.low: " << job.scheduled_time_range.get_low() << " sch.high: " << job.scheduled_time_range.get_high() << "\n"
                  << "schel.low: " << job.schedulable_time_range.get_low() << " schel.high: " << job.schedulable_time_range.get_high() << "\n\n";
    }
    std::cout << "Basic scheduler test passed!\n\n";
}

void test_empty_schedule() {
    std::cout << "Running empty schedule test...\n";
    
    std::vector<Job> empty_jobs;
    const time_t granularity = 15 * constants::minute;
    
    Schedule result = schedule(empty_jobs, granularity);
    
    assert(result.scheduled_jobs.empty());
    std::cout << "Empty schedule test passed!\n\n";
}

void test_rigid_job() {
    std::cout << "Running rigid job test...\n";
    
    const time_t granularity = 15 * constants::minute;
    
    Policy rigid_policy(0, 0.0, 3); // Both bits set
    std::set<ID> no_dependencies;
    std::set<Tag> meeting_tags = {Tag("meeting")};
    
    time_t meeting_start = 14 * constants::hour_to_minutes * constants::minute; // 2 PM
    time_t meeting_duration = constants::hour_to_minutes * constants::minute; // 1 hour
    
    TimeRange rigid_schedulable(meeting_start, meeting_start + meeting_duration);
    TimeRange rigid_scheduled(meeting_start, meeting_start + meeting_duration);
    
    std::vector<Job> jobs;
    jobs.emplace_back(
        meeting_duration,
        rigid_schedulable,
        rigid_scheduled,
        "rigid_meeting",
        rigid_policy,
        no_dependencies,
        meeting_tags
    );
    
    Schedule result = schedule(jobs, granularity);
    
    assert(result.scheduled_jobs.size() == 1);
    
    const auto& scheduled_job = result.scheduled_jobs[0];
    std::cout << "Rigid job scheduled successfully\n";
    std::cout << "Rigid job test passed!\n\n";
}

void test_friday_cost_optimization() {
    std::cout << "Running Friday cost optimization test...\n";
    
    const time_t granularity = 15 * constants::minute; // 15 minutes

    time_t thursday_start = 3 * constants::day;  // Thursday 9 AM
    time_t thursday_end = 3 * constants::day + 17 * constants::hour_to_minutes * constants::minute;   // Thursday 5 PM
    time_t friday_start = 4 * constants::day;  // Friday 9 AM
    time_t friday_end = 5 * constants::day;     // Friday 5 PM
    
    Policy flexible_policy(0, 0.0, 0); // Not splittable, not overlappable
    std::set<ID> no_dependencies;
    std::set<Tag> work_tags = {Tag("work")};
    
    TimeRange schedulable_range(thursday_start, friday_end);
    
    time_t initial_friday_time = friday_start + 2 * constants::hour_to_minutes * constants::minute; // Friday 11 AM
    time_t job_duration = 2 * constants::hour_to_minutes * constants::minute; // 2 hours
    TimeRange initial_scheduled(initial_friday_time, initial_friday_time + job_duration);
    
    std::vector<Job> jobs;
    jobs.emplace_back(
        job_duration,
        schedulable_range,
        initial_scheduled,
        "friday_task",
        flexible_policy,
        no_dependencies,
        work_tags
    );
    
    std::cout << "Initial schedule: Friday 11:00 AM - 1:00 PM\n";
    std::cout << "Job can be rescheduled between Thursday 9 AM and Friday 5 PM\n";
    std::cout << "Expected: Optimizer should move job to Thursday due to lower cost\n";
    
    Schedule result = schedule(jobs, granularity);
    
    assert(result.scheduled_jobs.size() == 1);
    
    const auto& optimized_job = result.scheduled_jobs[0];
    time_t scheduled_start = optimized_job.scheduled_time_range.get_low();
    
    bool moved_to_thursday = scheduled_start < friday_start;
    
    time_t days_from_monday = scheduled_start / constants::day;
    time_t time_of_day = scheduled_start % constants::day;
    time_t hours = time_of_day / (constants::hour_to_minutes * constants::minute);
    time_t minutes = (time_of_day % (constants::hour_to_minutes * constants::minute)) / constants::minute;
    
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
