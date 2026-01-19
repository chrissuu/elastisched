#include "engine.hpp"

#include "constants.hpp"
#include "policy.hpp"
#include "SimulatedAnnealingOptimizer.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <limits>
#include <map>
#include <optional>
#include <queue>
#include <random>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace {
template<typename T>
std::optional<T> safemin(std::optional<T> u, std::optional<T> v) {
    if (u.has_value() && v.has_value()) {
        return (u.value() < v.value() ? u : v);
    } else if (u.has_value()) {
        return u;
    } else if (v.has_value()) {
        return v;
    }
    return std::nullopt;
}

template<typename T>
std::optional<T> safemax(std::optional<T> u, std::optional<T> v) {
    if (u.has_value() && v.has_value()) {
        return (u.value() > v.value() ? u : v);
    } else if (u.has_value()) {
        return u;
    } else if (v.has_value()) {
        return v;
    }
    return std::nullopt;
}

std::vector<TimeRange> getJobScheduledRanges(const Job& job) {
    if (!job.scheduledTimeRanges.empty()) {
        return job.scheduledTimeRanges;
    }
    return {job.scheduledTimeRange};
}

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
} // namespace

Schedule::Schedule(std::vector<Job> scheduledJobs) : scheduledJobs(scheduledJobs) {}

void Schedule::addJob(const Job& job) {
    scheduledJobs.push_back(job);
}

void Schedule::clear() {
    scheduledJobs.clear();
}

std::ostream& operator<<(std::ostream& os, const Schedule& schedule) {
    const auto& jobs = schedule.scheduledJobs;
    os << "Schedule contains " << jobs.size() << " job(s):\n";
    for (const auto& job : jobs) {
        os << "  - Job Name: " << job.id << ", Scheduled Time: " << job.scheduledTimeRange << "\n";
    }
    return os;
}

DependencyViolation::DependencyViolation(ID id, const std::set<ID>& violations)
    : jobId(std::move(id)), violatedDependencies(violations) {}

DependencyCheckResult::DependencyCheckResult()
    : hasViolations(false), hasCyclicDependencies(false) {}

DependencyCheckResult checkDependencyViolations(const Schedule& schedule) {
    DependencyCheckResult result;

    if (schedule.scheduledJobs.empty()) {
        return result;
    }

    std::unordered_map<ID, const Job*> jobMap;
    std::unordered_map<ID, time_t> earliestStart;
    std::unordered_map<ID, time_t> latestEnd;
    for (const auto& job : schedule.scheduledJobs) {
        jobMap[job.id] = &job;
        time_t minStart = job.scheduledTimeRange.getLow();
        time_t maxEnd = job.scheduledTimeRange.getHigh();
        if (!job.scheduledTimeRanges.empty()) {
            minStart = job.scheduledTimeRanges.front().getLow();
            maxEnd = job.scheduledTimeRanges.front().getHigh();
            for (const auto& range : job.scheduledTimeRanges) {
                minStart = std::min(minStart, range.getLow());
                maxEnd = std::max(maxEnd, range.getHigh());
            }
        }
        earliestStart[job.id] = minStart;
        latestEnd[job.id] = maxEnd;
    }

    std::unordered_map<ID, std::vector<ID>> adjList;
    std::unordered_map<ID, int> inDegree;

    for (const auto& job : schedule.scheduledJobs) {
        inDegree[job.id] = 0;
        adjList[job.id] = std::vector<ID>();
    }

    for (const auto& job : schedule.scheduledJobs) {
        for (const ID& depId : job.dependencies) {
            if (jobMap.find(depId) != jobMap.end()) {
                adjList[depId].push_back(job.id);
                inDegree[job.id]++;
            }
        }
    }

    std::queue<ID> queue;
    std::vector<ID> topologicalOrder;

    for (const auto& pair : inDegree) {
        if (pair.second == 0) {
            queue.push(pair.first);
        }
    }

    while (!queue.empty()) {
        ID currentId = queue.front();
        queue.pop();
        topologicalOrder.push_back(currentId);

        for (ID neighborId : adjList[currentId]) {
            inDegree[neighborId]--;
            if (inDegree[neighborId] == 0) {
                queue.push(neighborId);
            }
        }
    }

    if (topologicalOrder.size() != schedule.scheduledJobs.size()) {
        result.hasCyclicDependencies = true;
        result.hasViolations = true;
        return result;
    }

    for (const auto& job : schedule.scheduledJobs) {
        std::set<ID> violatedDeps;

        for (const ID& depId : job.dependencies) {
            if (jobMap.find(depId) != jobMap.end()) {
                if (latestEnd[depId] > earliestStart[job.id]) {
                    violatedDeps.insert(depId);
                }
            }
        }

        if (!violatedDeps.empty()) {
            result.violations.emplace_back(job.id, violatedDeps);
            result.hasViolations = true;
        }
    }

    return result;
}

ScheduleCostFunction::ScheduleCostFunction(const Schedule& schedule, time_t granularity)
    :
m_schedule(schedule),
m_granularity(granularity)
{
    if (schedule.scheduledJobs.size() == 0) {
        return;
    }

    for (const auto& job : schedule.scheduledJobs) {
        for (const auto& range : getJobScheduledRanges(job)) {
            m_min = safemin<time_t>(m_min, range.getLow());
            m_max = safemax<time_t>(m_max, range.getHigh());
        }
    }

    TimeRange curr = TimeRange(0, constants::DAY - 1);
    m_dayBasedSchedule.insert(curr, std::nullopt);

    while (curr.getHigh() < m_max.value()) {
        time_t nextLow = curr.getHigh() + 1;
        TimeRange next = TimeRange(nextLow, nextLow + constants::DAY - 1);
        m_dayBasedSchedule.insert(next, std::nullopt);
        curr = next;
    }

    for (const auto& job : schedule.scheduledJobs) {
        for (const auto& range : getJobScheduledRanges(job)) {
            TimeRange currInterval = TimeRange(range.getLow()); // TimeRange representing unit of time
            std::optional<std::vector<Job>>* currDayJobs = m_dayBasedSchedule.searchValue(currInterval);
            if (currDayJobs) {
                currDayJobs->emplace().push_back(job);
            }
        }
    }
}

double ScheduleCostFunction::context_switch_cost() const {
    return 0.0f;
}

double ScheduleCostFunction::illegal_schedule_cost() const {
    const std::vector<Job>& scheduledJobs = m_schedule.scheduledJobs;
    IntervalTree<time_t, size_t> nonOverlappableJobs;

    for (size_t i = 0; i < scheduledJobs.size(); ++i) {
        const Job& curr = scheduledJobs[i];
        Policy currPolicy = curr.policy;

        for (const auto& range : getJobScheduledRanges(curr)) {
            if (!curr.schedulableTimeRange.contains(range)) {
                return constants::ILLEGAL_SCHEDULE_COST;
            }

            if (!currPolicy.isOverlappable()) {
                auto overlappingInterval = nonOverlappableJobs.searchOverlap(
                    range
                );

                if (overlappingInterval != nullptr) {
                    return constants::ILLEGAL_SCHEDULE_COST;
                }

                nonOverlappableJobs.insert(
                    range,
                    i
                );
            }
        }
    }

    DependencyCheckResult dependencyCheck = checkDependencyViolations(m_schedule);
    if (dependencyCheck.hasCyclicDependencies || dependencyCheck.hasViolations) {
        return constants::ILLEGAL_SCHEDULE_COST;
    }

    return 0.0f;
}

double ScheduleCostFunction::overlap_cost() const {
    const std::vector<Job>& scheduledJobs = m_schedule.scheduledJobs;
    if (scheduledJobs.size() < 2) {
        return 0.0f;
    }
    const double granularity = m_granularity > 0 ? static_cast<double>(m_granularity) : 1.0;
    IntervalTree<time_t, size_t> overlapTree;
    double cost = 0.0f;
    for (size_t i = 0; i < scheduledJobs.size(); ++i) {
        for (const auto& current : getJobScheduledRanges(scheduledJobs[i])) {
            const auto overlaps = overlapTree.findOverlapping(current);
            for (const auto* interval : overlaps) {
                cost += static_cast<double>(current.overlap_length(*interval)) / granularity;
            }
            overlapTree.insert(current, i);
        }
    }
    return cost;
}

double ScheduleCostFunction::split_cost() const {
    const std::vector<Job>& scheduledJobs = m_schedule.scheduledJobs;
    double cost = 0.0f;
    for (const auto& job : scheduledJobs) {
        const auto ranges = getJobScheduledRanges(job);
        if (ranges.size() > 1) {
            cost += (static_cast<double>(ranges.size() - 1) * constants::SPLIT_COST_FACTOR);
        }
    }
    return cost;
}

double ScheduleCostFunction::scheduleCost() const {
    double cost = illegal_schedule_cost() + overlap_cost() + split_cost();
    return cost;
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
    (void)disjointJobs;
    std::mt19937 gen(constants::rng_seed());

    Schedule initialSchedule = Schedule(jobs);

    ScheduleCostFunction initialCostFunction = ScheduleCostFunction(initialSchedule, GRANULARITY);
    (void)initialCostFunction;

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
