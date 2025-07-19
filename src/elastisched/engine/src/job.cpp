#include "job.hpp"

Job::Job(time_t duration, TimeRange schedulableTimeRange, TimeRange scheduledTimeRange,
        ID id, Policy policy, std::set<ID> dependencies, std::set<Tag> tags) 
:   duration(duration),
    schedulableTimeRange(schedulableTimeRange),
    scheduledTimeRange(scheduledTimeRange),
    id(id),
    policy(policy),
    dependencies(dependencies),
    tags(tags)
{
        return;
};

bool Job::isRigid() const {
    return duration == schedulableTimeRange.length();
};