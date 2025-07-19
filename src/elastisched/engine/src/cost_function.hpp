#ifndef COST_FUNCTION_H
#define COST_FUNCTION_H

#include "schedule.hpp"
#include "constants.hpp"
#include "utils/IntervalTree.hpp"

#include <optional>
#include <iostream>
#include <set>

class ScheduleCostFunction {
private:
    const Schedule& m_schedule;
    const time_t m_granularity;
    const time_t m_startEpoch;
    const std::set<Tag> m_restTags;
    IntervalTree<time_t, std::optional<std::vector<Job>>> m_dayBasedSchedule;
    std::optional<time_t> m_min = std::nullopt;
    std::optional<time_t> m_max = std::nullopt;

public:
    double busy_saturday_afternoon_cost() const;
    double busy_friday_afternoon_cost() const;
    // double work_block_duration_cost() const;
    // double daily_work_load_balance() const;
    // double bad_sleep_cost() const;
    // double context_switch_cost() const;
    // double finish_later_cost() const;
    // double priority_inversion_cost() const;

    double scheduleCost() const;
    
    ScheduleCostFunction(const Schedule& schedule, time_t granularity, time_t startEpoch);
};
#endif