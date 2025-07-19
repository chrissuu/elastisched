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

ScheduleCostFunction::ScheduleCostFunction(const Schedule& schedule, time_t granularity, time_t startEpoch)
    : 
m_schedule(schedule),
m_granularity(granularity), 
m_startEpoch(startEpoch) { 
    if (schedule.scheduledJobs.size() == 0) return;

    for (const auto& job : schedule.scheduledJobs) {
        m_min = safemin<time_t>(m_min, job.scheduledTimeRange.getLow());
        m_max = safemax<time_t>(m_max, job.scheduledTimeRange.getHigh());
    }

    TimeRange curr = TimeRange(startEpoch, startEpoch + constants::DAY - 1);
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


double ScheduleCostFunction::busy_friday_afternoon_cost() const {
    time_t TIME_TO_FIRST_FRIDAY = m_startEpoch + 4 * constants::DAY;
    TimeRange currFriday = TimeRange(TIME_TO_FIRST_FRIDAY);
    std::optional<std::vector<Job>>* currFridayJobs = m_dayBasedSchedule.searchValue(currFriday);
    double cost = 0;
    while (currFriday.getHigh() < m_max) {
        if (currFridayJobs->has_value()) {
            cost += constants::FRIDAY_HOURLY_COST_FACTOR * (double)getTotalJobsLength(currFridayJobs->value());
        }
        currFriday = TimeRange(currFriday.getHigh() + constants::WEEK);
        currFridayJobs = m_dayBasedSchedule.searchValue(currFriday);
    }
    return cost;
}


double ScheduleCostFunction::busy_saturday_afternoon_cost() const {
    time_t TIME_TO_FIRST_SAT = m_startEpoch + 5 * constants::DAY;
    TimeRange currSat = TimeRange(TIME_TO_FIRST_SAT);
    std::optional<std::vector<Job>>* currSatJobs = m_dayBasedSchedule.searchValue(currSat);
    double cost = 0;
    while (currSat.getHigh() < m_max) {
        if (currSatJobs->has_value()) {
            cost += constants::SATURDAY_HOURLY_COST_FACTOR * (double)getTotalJobsLength(currSatJobs->value());
        }
        currSat = TimeRange(currSat.getHigh() + constants::WEEK);
        currSatJobs = m_dayBasedSchedule.searchValue(currSat);
    }
    return cost;
}

double ScheduleCostFunction::scheduleCost() const {
    double cost = busy_friday_afternoon_cost() + busy_saturday_afternoon_cost();
    return cost;
}