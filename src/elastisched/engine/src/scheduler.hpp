#ifndef SCHEDULER_H
#define SCHEDULER_H

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
#include <fstream>
#include <set>
#include <cassert>
#include <utility>

#include "optimizer/SimulatedAnnealingOptimizer.hpp"

Schedule schedule(std::vector<Job> jobs, const uint64_t GRANULARITY);
std::pair<Schedule, std::vector<double>> scheduleJobs(std::vector<Job> jobs, const uint64_t GRANULARITY, const double INITIAL_TEMP, const double FINAL_TEMP, const uint64_t NUM_ITERS);

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

            job.scheduledTimeRange = randomTimeRange;
            job.setScheduledTimeRanges({randomTimeRange});
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

bool rangesOverlap(const TimeRange& candidate, const std::vector<TimeRange>& ranges) {
    for (const auto& range : ranges) {
        if (candidate.overlaps(range)) {
            return true;
        }
    }
    return false;
}

std::vector<time_t> generateSplitDurations(
    time_t duration,
    size_t segmentCount,
    time_t minSplitDuration,
    time_t granularity,
    bool roundToGranularity,
    std::mt19937& gen
) {
    if (segmentCount <= 1) {
        return {duration};
    }

    time_t unit = 1;
    if (roundToGranularity && granularity > 0 && duration % granularity == 0) {
        unit = granularity;
    } else {
        roundToGranularity = false;
    }

    time_t minSplit = minSplitDuration > 0 ? minSplitDuration : 1;
    if (roundToGranularity && unit > 1) {
        minSplit = ((minSplit + unit - 1) / unit) * unit;
    }

    if (minSplit * segmentCount > duration) {
        return {};
    }

    std::vector<time_t> durations(segmentCount, minSplit);
    time_t remaining = duration - minSplit * segmentCount;

    if (roundToGranularity && unit > 1) {
        if (remaining % unit != 0) {
            return {};
        }
        size_t increments = remaining / unit;
        std::uniform_int_distribution<size_t> dist(0, segmentCount - 1);
        for (size_t i = 0; i < increments; ++i) {
            durations[dist(gen)] += unit;
        }
        return durations;
    }

    if (remaining > 0) {
        std::vector<time_t> cuts;
        cuts.reserve(segmentCount + 1);
        std::uniform_int_distribution<time_t> dist(0, remaining);
        cuts.push_back(0);
        cuts.push_back(remaining);
        for (size_t i = 0; i < segmentCount - 1; ++i) {
            cuts.push_back(dist(gen));
        }
        std::sort(cuts.begin(), cuts.end());
        for (size_t i = 0; i < segmentCount; ++i) {
            durations[i] += (cuts[i + 1] - cuts[i]);
        }
    }
    return durations;
}

std::vector<TimeRange> placeSplitSegments(
    const TimeRange& schedulableTimeRange,
    const std::vector<time_t>& durations,
    time_t granularity,
    std::mt19937& gen
) {
    std::vector<TimeRange> segments;
    std::vector<time_t> durationsCopy = durations;
    std::shuffle(durationsCopy.begin(), durationsCopy.end(), gen);
    for (const auto& duration : durationsCopy) {
        bool placed = false;
        const int maxAttempts = 50;
        for (int attempt = 0; attempt < maxAttempts; ++attempt) {
            TimeRange candidate = generateRandomTimeRangeWithin(
                schedulableTimeRange,
                duration,
                granularity,
                gen
            );
            if (!rangesOverlap(candidate, segments)) {
                segments.push_back(candidate);
                placed = true;
                break;
            }
        }
        if (!placed) {
            return {};
        }
    }
    std::sort(segments.begin(), segments.end(), [](const TimeRange& a, const TimeRange& b) {
        return a.getLow() < b.getLow();
    });
    return segments;
}

Schedule generateRandomScheduleNeighbor(
    Schedule s, 
    const time_t GRANULARITY,
    std::mt19937& gen
) {
    std::vector<Job>& jobs = s.scheduledJobs;
    std::vector<size_t> flexibleIndices;

    for (size_t i = 0; i < jobs.size(); ++i) {
        if (!jobs[i].isRigid()) {
            flexibleIndices.push_back(i);
        }
    }

    if (flexibleIndices.empty()) {
        return s;
    }

    std::uniform_int_distribution<> dist(0, flexibleIndices.size() - 1);
    size_t chosenIndex = flexibleIndices[dist(gen)];

    Job& randomFlexibleJob = jobs[chosenIndex];
    Policy policy = randomFlexibleJob.policy;
    bool canSplit = policy.isSplittable() && policy.getMaxSplits() > 0;
    time_t minSplitDuration = policy.getMinSplitDuration();
    time_t minSplit = minSplitDuration > 0 ? minSplitDuration : 1;
    bool roundToGranularity = policy.getRoundToGranularity()
        && GRANULARITY > 0
        && (randomFlexibleJob.duration % GRANULARITY == 0);
    if (roundToGranularity && GRANULARITY > 1) {
        minSplit = ((minSplit + GRANULARITY - 1) / GRANULARITY) * GRANULARITY;
    }
    size_t maxSegments = static_cast<size_t>(policy.getMaxSplits()) + 1;
    size_t maxSegmentsByDuration = static_cast<size_t>(randomFlexibleJob.duration / minSplit);
    size_t possibleSegments = std::min(maxSegments, maxSegmentsByDuration);

    bool attemptSplit = false;
    if (canSplit && possibleSegments >= 2) {
        std::uniform_int_distribution<int> splitDecision(0, 1);
        attemptSplit = (splitDecision(gen) == 1);
    }

    if (attemptSplit) {
        std::uniform_int_distribution<size_t> splitCountDist(2, possibleSegments);
        size_t segmentCount = splitCountDist(gen);
        std::vector<time_t> splitDurations = generateSplitDurations(
            randomFlexibleJob.duration,
            segmentCount,
            minSplitDuration,
            GRANULARITY,
            roundToGranularity,
            gen
        );

        if (!splitDurations.empty()) {
            std::vector<TimeRange> splitRanges = placeSplitSegments(
                randomFlexibleJob.schedulableTimeRange,
                splitDurations,
                GRANULARITY,
                gen
            );
            if (!splitRanges.empty()) {
                randomFlexibleJob.setScheduledTimeRanges(splitRanges);
                return s;
            }
        }
    }

    TimeRange randomTimeRange = generateRandomTimeRangeWithin(
        randomFlexibleJob.schedulableTimeRange,
        randomFlexibleJob.duration,
        GRANULARITY,
        gen
    );

    randomFlexibleJob.setScheduledTimeRanges({randomTimeRange});

    return s;
}

/**
 * 
 * @param rigid := a linked list containing nodes which cannot be moved
 * @param flexible := a linked list containing all flexible nodes
 * @param GRANULARITY := the smallest schedulable delta
 * 
 * Returns the approximately best Schedule.
 * 
 */
std::pair<Schedule, std::vector<double>> scheduleJobs(
    std::vector<Job> jobs,
    const time_t GRANULARITY, 
    const double INITIAL_TEMP,
    const double FINAL_TEMP,
    const uint64_t NUM_ITERS
) {
    if (jobs.size() == 0) {
        return std::make_pair<Schedule, std::vector<double>>(Schedule(), {});
    };

    for (auto& job : jobs) {
        if (job.isRigid()) {
            job.scheduledTimeRange = job.schedulableTimeRange;
            job.setScheduledTimeRanges({job.scheduledTimeRange});
        }
    }

    std::vector<std::vector<Job>> disjointJobs = getDisjointIntervals(jobs);
    std::mt19937 gen(constants::rng_seed());

    Schedule initialSchedule = Schedule(jobs);

    ScheduleCostFunction initialCostFunction = ScheduleCostFunction(initialSchedule, GRANULARITY);

    SimulatedAnnealingOptimizer<Schedule> optimizer = SimulatedAnnealingOptimizer<Schedule>(
        [GRANULARITY](Schedule s) {
            ScheduleCostFunction costFunction = ScheduleCostFunction(s, GRANULARITY);
            return costFunction.scheduleCost();
        },
        [GRANULARITY, &gen](Schedule s) { 
            return generateRandomScheduleNeighbor(
                s, 
                GRANULARITY,
                gen); 
        },
        INITIAL_TEMP,
        FINAL_TEMP,
        NUM_ITERS
    );

    Schedule bestSchedule = optimizer.optimize(initialSchedule);
    std::vector<double> costHistory = optimizer.getCostHistory();

    return std::make_pair(bestSchedule, costHistory);
}

Schedule schedule(
    std::vector<Job> jobs, 
    const uint64_t GRANULARITY
) {
    std::pair<Schedule, std::vector<double>> s = scheduleJobs(
        jobs, 
        GRANULARITY, 
        10.0f, 
        1e-4, 
        1000000
    );

    return s.first;
}


#endif
