#ifndef JOB_HPP
#define JOB_HPP

#include "policy.hpp"
#include "tag.hpp"
#include "constants.hpp"
#include "utils/interval.hpp"

#include <vector>
#include <set>

class Job {
public:
    TimeRange defaultScheduledTimeRange;
    TimeRange schedulableTimeRange;
    ID id;
    Policy policy;
    std::vector<ID> dependencies;
    std::set<Tag> tags;

    // Constructor
    Job(TimeRange defaultScheduledTimeRange,
        TimeRange schedulableTimeRange,
        ID id,
        Policy policy,
        std::vector<ID> dependencies,
        std::set<Tag> tags);
};

#endif // JOB_HPP
