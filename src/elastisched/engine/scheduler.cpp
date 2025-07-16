#include "job.hpp"
#include "policy.hpp"
#include "schedule.hpp"
#include "utils/interval_tree.hpp"

#include "constants.hpp"

#include <vector>
#include <memory>
#include <algorithm>
#include <cstddef>
#include <random>

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

std::vector<std::vector<std::unique_ptr<Job>>> getDisjointIntervals(std::vector<std::unique_ptr<Job>> jobs) {
    if (jobs.empty()) {
        return {};
    }

    std::vector<std::vector<std::unique_ptr<Job>>> disjointIntervals;

    std::sort(jobs.begin(), jobs.end(), [](const std::unique_ptr<Job>& a, const std::unique_ptr<Job>& b) {
        return a->schedulableTimeRange.low() < b->schedulableTimeRange.low();
    });

    time_t currEnd = jobs[0]->schedulableTimeRange.high();
    disjointIntervals.push_back({std::move(jobs[0])});

    for (size_t i = 1; i < jobs.size(); ++i) {
        auto& job = jobs[i];

        if (job->schedulableTimeRange.low() >= currEnd) {
            disjointIntervals.push_back({std::move(job)});
            currEnd = disjointIntervals.back().back()->schedulableTimeRange.high();
        } else {
            disjointIntervals.back().push_back(std::move(job));
            currEnd = std::max(currEnd, disjointIntervals.back().back()->schedulableTimeRange.high());
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
    std::vector<std::vector<std::unique_ptr<Job>>> disjointJobs, 
    time_t GRANULARITY,
    std::mt19937& gen
) {
    Schedule s;
    std::vector<ScheduledJob> randomScheduledJobs;
    
    for (auto jobGroup : disjointJobs) {
        for (const auto& job : jobGroup) {
            TimeRange schedulableTimeRange = job->schedulableTimeRange;
            time_t duration = job->defaultScheduledTimeRange.length();
            TimeRange randomTimeRange = generateRandomTimeRangeWithin(
                schedulableTimeRange,
                duration,
                GRANULARITY,    
                gen
            );

            ScheduledJob randomScheduledJob = { job->tags, randomTimeRange, job->id };

            randomScheduledJobs.push_back(randomScheduledJob);
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
std::optional<Schedule> schedule_jobs(
    std::vector<std::unique_ptr<Job>> jobs,
    const time_t GRANULARITY, 
    const time_t START_EPOCH,
    const uint64_t NUM_SAMPLES
) {
    IntervalTree<time_t> rigidIntervals;
    for (const auto& job : jobs) {
        if (job->defaultScheduledTimeRange == job->schedulableTimeRange) {
            rigidIntervals.insert(job->defaultScheduledTimeRange);
        }
    }

    auto minIntervalTimeOpt = getMinIntervalTime(jobs);
    auto maxIntervalTimeOpt = getMaxIntervalTime(jobs);

    std::vector<std::vector<std::unique_ptr<Job>>> disjointJobs = getDisjointIntervals(jobs);

    std::vector<Schedule> randomSchedules;

    std::random_device rd;
    std::mt19937 gen(rd());

    for (auto i = 0; i < NUM_SAMPLES; i++) {
        randomSchedules.push_back(generateRandomSchedule(disjointJobs, GRANULARITY, gen));
    }

    return std::nullopt;
}

int schedule(std::vector<std::unique_ptr<Job>> jobs, uint8_t num_jobs, const uint64_t GRANULARITY, const uint64_t START_EPOCH) {
    if (num_jobs == 0) {
        return 1;
    }

    schedule_jobs(jobs, GRANULARITY, START_EPOCH, 1UL);
}