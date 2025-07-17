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

#include "optimizer/SimulatedAnnealingOptimizer.hpp"

std::optional<time_t> getMaxIntervalTime(const std::vector<std::unique_ptr<Job>>& jobs) {
    if (jobs.empty()) return std::nullopt;

    std::optional<time_t> currMax = std::nullopt;
    for (const auto& job : jobs) {
        time_t start = job->schedulableTimeRange.low();
        if (!currMax || start > *currMax) {
            currMax = start;
        }
    }
    return currMax;
}

std::optional<time_t> getMinIntervalTime(const std::vector<std::unique_ptr<Job>>& jobs) {
    if (jobs.empty()) return std::nullopt;

    std::optional<time_t> currMin = std::nullopt;
    for (const auto& job : jobs) {
        time_t start = job->schedulableTimeRange.low();
        if (!currMin || start < *currMin) {
            currMin = start;
        }
    }
    return currMin;
}

std::optional<time_t> optionalMin(std::optional<time_t> a, std::optional<time_t> b) {
    if (a && b) return std::min(*a, *b);
    else if (a) return a;
    else return b;
}

std::optional<time_t> optionalMax(std::optional<time_t> a, std::optional<time_t> b) {
    if (a && b) return std::max(*a, *b);
    else if (a) return a;
    else return b;
}

std::vector<std::vector<Job>> getDisjointIntervals(std::vector<Job> jobs) {
    if (jobs.empty()) {
        return {};
    }

    std::vector<std::vector<Job>> disjointIntervals;

    std::sort(jobs.begin(), jobs.end(), [](const std::unique_ptr<Job>& a, const std::unique_ptr<Job>& b) {
        return a->schedulableTimeRange.low() < b->schedulableTimeRange.low();
    });

    time_t currEnd = jobs[0].schedulableTimeRange.high();
    disjointIntervals.push_back({jobs[0]});

    for (size_t i = 1; i < jobs.size(); ++i) {
        auto& job = jobs[i];

        if (job.schedulableTimeRange.low() >= currEnd) {
            disjointIntervals.push_back({job});
            currEnd = disjointIntervals.back().back().schedulableTimeRange.high();
        } else {
            disjointIntervals.back().push_back(std::move(job));
            currEnd = std::max(currEnd, disjointIntervals.back().back().schedulableTimeRange.high());
        }
    }

    return disjointIntervals;
}


TimeRange generateRandomTimeRangeWithin(
    const TimeRange& schedulableTimeRange,
    time_t duration,
    time_t GRANULARITY,
    const std::mt19937& gen)
{
    time_t earliestStart = ((schedulableTimeRange.low() + GRANULARITY - 1) / GRANULARITY) * GRANULARITY;

    time_t rawLatestStart = schedulableTimeRange.high() - duration;

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
    std::vector<ScheduledJob> randomScheduledJobs;
    
    for (auto jobGroup : disjointJobs) {
        for (const auto& job : jobGroup) {
            TimeRange schedulableTimeRange = job.schedulableTimeRange;
            time_t duration = job.defaultScheduledTimeRange.length();
            TimeRange randomTimeRange = generateRandomTimeRangeWithin(
                schedulableTimeRange,
                duration,
                GRANULARITY,    
                gen
            );

            ScheduledJob randomScheduledJob = { job.tags, randomTimeRange, job.id };

            randomScheduledJobs.push_back(randomScheduledJob);
        }
    }
}

template<typename T>
T randomChoice(const std::vector<T>& vec, const std::mt19937& gen) {
    std::uniform_int_distribution<> dist(0, vec.size() - 1);
    return vec[dist(gen)];
}

Schedule generateRandomScheduleNeighbor(
    Schedule s, 
    const IntervalTree<time_t>& rigidIntervals,
    std::map<ID, Job> idBlobMap,
    const time_t GRANULARITY,
    const std::mt19937& gen
) {
    std::vector<ScheduledJob> scheduledJobs = s.scheduledJobs;
    for (;;) {
        ScheduledJob randomScheduledJob = randomChoice(scheduledJobs, gen);
        ID currId = randomScheduledJob.id;
        Job currJob = idBlobMap[currId];
        if (!currJob.isRigid()) {
            TimeRange randomTimeRange = generateRandomTimeRangeWithin(
                currJob.schedulableTimeRange,
                currJob.schedulableTimeRange.length(),
                GRANULARITY,
                gen
            );
        }
    }
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
    const time_t START_EPOCH,
    const uint64_t NUM_SAMPLES
) {
    IntervalTree<time_t> rigidIntervals;
    std::map<ID, Job> idBlobMap;
    for (const auto& job : jobs) {
        if (job.defaultScheduledTimeRange == job.schedulableTimeRange) {
            rigidIntervals.insert(job.defaultScheduledTimeRange);
        }
        idBlobMap[job.id] = job;
    }

    std::vector<std::vector<Job>> disjointJobs = getDisjointIntervals(jobs);
    std::random_device rd;
    std::mt19937 gen(rd());

    Schedule randomSchedule = generateRandomSchedule(disjointJobs, GRANULARITY, gen);

    SimulatedAnnealingOptimizer<Schedule> optimizer = SimulatedAnnealingOptimizer<Schedule>(
        scheduleCost,
        [rigidIntervals, idBlobMap, GRANULARITY, gen](Schedule s) { 
            return generateRandomScheduleNeighbor(
                s, 
                rigidIntervals, 
                idBlobMap,
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

    schedule_jobs(jobs, GRANULARITY, START_EPOCH, 1UL);
}