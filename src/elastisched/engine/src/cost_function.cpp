#include "cost_function.hpp"

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

/**
 * getTotalJobsLength
 * 
 * @param jobs
 * 
 * @returns Sum of all TimeRange lengths of all jobs in the vector
 */
time_t getTotalJobsLength(std::vector<Job> jobs) {
    time_t length = 0;
    for (const auto& job : jobs) {
        length += job.scheduledTimeRange.length();
    }
    return length;
}

ScheduleCostFunction::ScheduleCostFunction(const Schedule& schedule, time_t granularity)
    : 
m_schedule(schedule),
m_granularity(granularity)
{ 
    if (schedule.scheduledJobs.size() == 0) return;

    for (const auto& job : schedule.scheduledJobs) {
        m_min = safemin<time_t>(m_min, job.scheduledTimeRange.getLow());
        m_max = safemax<time_t>(m_max, job.scheduledTimeRange.getHigh());
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
        TimeRange currInterval = TimeRange(job.scheduledTimeRange.getLow()); // TimeRange representing unit of time
        std::optional<std::vector<Job>>* currDayJobs = m_dayBasedSchedule.searchValue(currInterval);
        if (currDayJobs) {
            currDayJobs->emplace().push_back(job);
        }
    }
}


/**
 * Adds a cost to busy afternoons, exponentially increasing cost for jobs appearing later in the day.
 */
double ScheduleCostFunction::busy_afternoon_exponential_cost(uint64_t DAYS_SINCE_MONDAY) const {
    time_t TIME_TO_FIRST_DAY = DAYS_SINCE_MONDAY * constants::DAY;
    TimeRange currDay = TimeRange(TIME_TO_FIRST_DAY);
    std::optional<std::vector<Job>>* currJobs = m_dayBasedSchedule.searchValue(currDay);

    double cost = 0;
    while (currDay.getHigh() < m_max.value()) {
        if (currJobs->has_value()) {
            std::vector<Job> filteredWorkJobs;
            for (const auto& job : currJobs->value()) {
                bool is_work_type = job.tags.find(constants::WORK_TAG) != job.tags.end();
                /* scheduledTimeRange.low := DAY * day (since EPOCH) + HOUR * hour + MINUTES * minute + second */
                bool is_scheduled_in_afternoon = ((job.scheduledTimeRange.getLow() / constants::HOUR) % 24) >= constants::AFTERNOON_START;
                if (is_work_type && is_scheduled_in_afternoon) {
                    filteredWorkJobs.push_back(job);
                }
            }
            
            /* Apply greater cost for work type jobs scheduled later in the afternoon */
            for (const auto& job : filteredWorkJobs) {
                cost += std::exp(constants::EXP_DOWNFACTOR * ((double)(job.scheduledTimeRange.length()) / constants::HOUR));
            }
        }
        currDay = TimeRange(currDay.getHigh() + constants::WEEK);
        currJobs = m_dayBasedSchedule.searchValue(currDay);
    }
    return cost;    
}


/**
 * Adds a cost to busy days, increasing by constant cost for each hour scheduled on that day
 */
double ScheduleCostFunction::busy_day_constant_cost(uint64_t DAYS_SINCE_MONDAY) const {
    time_t TIME_TO_FIRST_DAY = DAYS_SINCE_MONDAY * constants::DAY;
    TimeRange currDay = TimeRange(TIME_TO_FIRST_DAY);
    std::optional<std::vector<Job>>* currJobs = m_dayBasedSchedule.searchValue(currDay);
    double cost = 0;
    while (currDay.getHigh() < m_max.value()) {
        if (currJobs->has_value()) {
            std::vector<Job> filteredWorkJobs;
            for (const auto& job : currJobs->value()) {
                bool is_work_type = job.tags.find(constants::WORK_TAG) != job.tags.end();
                if (is_work_type) {
                    filteredWorkJobs.push_back(job);
                }
            }
            
            for (const auto& job : filteredWorkJobs) {
                cost += constants::HOURLY_COST_FACTOR * ((double)(job.scheduledTimeRange.length()) / constants::HOUR);
            }
        }
        currDay = TimeRange(currDay.getHigh() + constants::WEEK);
        currJobs = m_dayBasedSchedule.searchValue(currDay);
    }
    return cost;    
}


/**
 * Adds a cost for scheduling tasks consecutively which are too different from each other
 */
double ScheduleCostFunction::context_switch_cost() const {
    return 0.0f;
}

double ScheduleCostFunction::busy_friday_afternoon_cost() const {
    return busy_afternoon_exponential_cost((uint64_t)4);
}


double ScheduleCostFunction::busy_saturday_cost() const {
    return busy_day_constant_cost((uint64_t)5);
}


/**
 * Adds an "INFINITE" cost to illegal schedulings.
 */
double ScheduleCostFunction::illegal_schedule_cost() const {
    const std::vector<Job>& scheduledJobs = m_schedule.scheduledJobs;
    IntervalTree<time_t, size_t> nonOverlappableJobs;

    for (size_t i = 0; i < scheduledJobs.size(); ++i) {
        const Job& curr = scheduledJobs[i];
        Policy currPolicy = curr.policy;

        if (!curr.schedulableTimeRange.contains(curr.scheduledTimeRange)) {
            return constants::ILLEGAL_SCHEDULE_COST;
        }
        
        if (!currPolicy.isOverlappable()) {
            auto overlappingInterval = nonOverlappableJobs.searchOverlap(
                curr.scheduledTimeRange
            );
            
            if (overlappingInterval != nullptr) {
                return constants::ILLEGAL_SCHEDULE_COST;
            }
            
            nonOverlappableJobs.insert(
                curr.scheduledTimeRange,
                i
            );
        }
    }
    
    DependencyCheckResult dependencyCheck = checkDependencyViolations(m_schedule);
    if (dependencyCheck.hasCyclicDependencies || dependencyCheck.hasViolations) {
        return constants::ILLEGAL_SCHEDULE_COST;
    }

    return 0.0f;
}

/**
 * @brief Adds a cost which helps reduce the amount of overlap that a schedule will accept
 * 
 * @return double 
 */
double ScheduleCostFunction::overlap_cost() const {
    const std::vector<Job>& scheduledJobs = m_schedule.scheduledJobs;
    if (scheduledJobs.size() < 2) {
        return 0.0f;
    }
    const double granularity = m_granularity > 0 ? static_cast<double>(m_granularity) : 1.0;
    IntervalTree<time_t, size_t> overlapTree;
    double cost = 0.0f;
    for (size_t i = 0; i < scheduledJobs.size(); ++i) {
        const TimeRange& current = scheduledJobs[i].scheduledTimeRange;
        const auto overlaps = overlapTree.findOverlapping(current);
        for (const auto* interval : overlaps) {
            cost += static_cast<double>(current.overlap_length(*interval)) / granularity;
        }
        overlapTree.insert(current, i);
    }
    return cost;
}


double ScheduleCostFunction::scheduleCost() const {
    double cost = illegal_schedule_cost() + overlap_cost();
    return cost;
}
