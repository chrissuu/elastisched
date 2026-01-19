#ifndef JOB_HPP
#define JOB_HPP

#include "Policy.hpp"
#include "Tag.hpp"
#include "constants.hpp"
#include "types.hpp"
#include "Interval.hpp"

#include <vector>
#include <set>
#include <iostream>
#include <sstream>

class Job {
public:
    sec_t duration;
    TimeRange schedulable_time_range;
    TimeRange scheduled_time_range;
    std::vector<TimeRange> scheduled_time_ranges;
    ID id;
    Policy policy;
    std::set<ID> dependencies;
    std::set<Tag> tags;

    Job(sec_t duration,
        TimeRange schedulable_time_range,
        TimeRange scheduled_time_range,
        ID id,
        Policy policy,
        std::set<ID> dependencies,
        std::set<Tag> tags);

    bool is_rigid() const;
    const std::vector<TimeRange>& get_scheduled_time_ranges() const;
    void set_scheduled_time_ranges(std::vector<TimeRange> ranges);
    std::string to_string() const;
};

#endif // JOB_HPP
