#include "job.hpp"
#include "policy.hpp"
#include "schedule.hpp"
#include "utils/IntervalTree.hpp"
#include "cost_function.hpp"

#include "constants.hpp"

#include <vector>
#include <memory>
#include <algorithm>
#include <cstddef>
#include <map>
#include <random>
#include <optional>
#include <iostream>

#include "optimizer/SimulatedAnnealingOptimizer.hpp"

std::vector<std::vector<Job>> getDisjointIntervals(std::vector<Job> jobs) {
    if (jobs.empty()) {
        return {};
    }

    std::vector<std::vector<Job>> disjointIntervals;

    std::sort(jobs.begin(), jobs.end(), [](const Job& a, const Job& b) {
        return a.schedulableTimeRange.getLow() < b.schedulableTimeRange.getLow();
    });

    time_t currEnd = jobs[0].schedulableTimeRange.getHigh();
    disjointIntervals.push_back({jobs[0]});

    for (size_t i = 1; i < jobs.size(); ++i) {
        auto& job = jobs[i];

        if (job.schedulableTimeRange.getLow() >= currEnd) {
            disjointIntervals.push_back({job});
            currEnd = disjointIntervals.back().back().schedulableTimeRange.getHigh();
        } else {
            disjointIntervals.back().push_back(std::move(job));
            currEnd = std::max(currEnd, disjointIntervals.back().back().schedulableTimeRange.getHigh());
        }
    }

    return disjointIntervals;
}


TimeRange generateRandomTimeRangeWithin(
    const TimeRange& schedulableTimeRange,
    time_t duration,
    time_t GRANULARITY,
    std::mt19937& gen)
{
    time_t earliestStart = ((schedulableTimeRange.getLow() + GRANULARITY - 1) / GRANULARITY) * GRANULARITY;

    time_t rawLatestStart = schedulableTimeRange.getHigh() - duration;

    time_t latestStart = (rawLatestStart / GRANULARITY) * GRANULARITY;

    if (latestStart < earliestStart) {
        throw std::invalid_argument("Schedulable timerange too small for the job duration");
    }

    size_t numSlots = (latestStart - earliestStart) / GRANULARITY + 1;

    std::uniform_int_distribution<size_t> dis(0, numSlots - 1);
    size_t randomSlot = dis(gen);

    time_t start = earliestStart + randomSlot * GRANULARITY;
    return TimeRange(start, start + duration);
}


Schedule generateRandomSchedule(
    std::vector<std::vector<Job>> disjointJobs, 
    time_t GRANULARITY,
    std::mt19937& gen
) {
    Schedule s;
    std::vector<Job> randomlyScheduledJobs;
    
    for (auto jobGroup : disjointJobs) {
        for (auto job : jobGroup) {
            TimeRange schedulableTimeRange = job.schedulableTimeRange;
            time_t duration = job.duration;
            TimeRange randomTimeRange = generateRandomTimeRangeWithin(
                schedulableTimeRange,
                duration,
                GRANULARITY,    
                gen
            );

            job.schedulableTimeRange = randomTimeRange;
            randomlyScheduledJobs.push_back(job);
        }
    }
    s.scheduledJobs = randomlyScheduledJobs;
    return s;
}

template<typename T>
T* randomChoice(std::vector<T>& vec, std::mt19937& gen) {
    std::uniform_int_distribution<> dist(0, vec.size() - 1);
    return &vec[dist(gen)];
}

Schedule generateRandomScheduleNeighbor(
    Schedule s, 
    const time_t GRANULARITY,
    std::mt19937& gen
) {
    std::vector<Job> jobs = s.scheduledJobs;
    std::vector<Job> flexibleJobs;
    for (auto& job : jobs) {
        if (!job.isRigid()) {
            flexibleJobs.push_back(job);
        }
    }

    if (flexibleJobs.size() == 0) {
        return s;
    }

    Job* randomFlexibleJob = randomChoice(flexibleJobs, gen);
    TimeRange randomTimeRange = generateRandomTimeRangeWithin(
        randomFlexibleJob->schedulableTimeRange,
        randomFlexibleJob->schedulableTimeRange.length(),
        GRANULARITY,
        gen
    );
    randomFlexibleJob->scheduledTimeRange = randomTimeRange;

    return s;
}

/**
 * 
 * @param rigid := a linked list containing nodes which cannot be moved
 * @param flexible := a linked list containing all flexible nodes
 * @param GRANULARITY := the smallest schedulable delta
 * @param START_EPOCH := the time (in seconds) since an epoch which indicates 
 *                       the start of the most recent Sunday (timezone agnostic)
 * 
 * Returns the approximately best Schedule.
 * 
 */
Schedule schedule_jobs(
    std::vector<Job> jobs,
    const time_t GRANULARITY, 
    const time_t START_EPOCH
) {
    std::vector<std::vector<Job>> disjointJobs = getDisjointIntervals(jobs);
    std::random_device rd;
    std::mt19937 gen(rd());

    Schedule randomSchedule = generateRandomSchedule(disjointJobs, GRANULARITY, gen);

    SimulatedAnnealingOptimizer<Schedule> optimizer = SimulatedAnnealingOptimizer<Schedule>(
        [GRANULARITY, START_EPOCH](Schedule s) {
            ScheduleCostFunction costFunction = ScheduleCostFunction(s, GRANULARITY, START_EPOCH);
            return costFunction.scheduleCost();
        },
        [GRANULARITY, &gen](Schedule s) { 
            return generateRandomScheduleNeighbor(
                s, 
                GRANULARITY,
                gen); },
        100.0f,
        1e-4,
        100000
    );

    Schedule bestSchedule = optimizer.optimize(randomSchedule);

    return bestSchedule;
}

Schedule schedule(std::vector<Job> jobs, uint8_t num_jobs, const uint64_t GRANULARITY, const uint64_t START_EPOCH) {
    if (num_jobs == 0) {
        return Schedule();
    }

    Schedule s = schedule_jobs(jobs, GRANULARITY, START_EPOCH);
    return s;
}

int main(int argc, char** argv) {
    std::cout << "Hello World!";
    return 0;
}