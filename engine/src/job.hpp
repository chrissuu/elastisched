#ifndef JOB_HPP
#define JOB_HPP

#include "policy.hpp"
#include "tag.hpp"
#include "constants.hpp"
#include "Interval.hpp"

#include <vector>
#include <set>
#include <iostream>
#include <sstream>

class Job {
public:
    time_t duration;
    TimeRange schedulableTimeRange;
    TimeRange scheduledTimeRange;
    std::vector<TimeRange> scheduledTimeRanges;
    ID id;
    Policy policy;
    std::set<ID> dependencies;
    std::set<Tag> tags;

    Job(time_t duration,
        TimeRange schedulableTimeRange,
        TimeRange scheduledTimeRange,
        ID id,
        Policy policy,
        std::set<ID> dependencies,
        std::set<Tag> tags);

    bool isRigid() const;
    const std::vector<TimeRange>& getScheduledTimeRanges() const;
    void setScheduledTimeRanges(std::vector<TimeRange> ranges);
    std::string toString() const;
};

#endif // JOB_HPP
