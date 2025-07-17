#include "job.hpp"

Job::Job(TimeRange defaultScheduledTimeRange, TimeRange schedulableTimeRange,
        ID id, Policy policy, std::set<ID> dependencies, std::set<Tag> tags) 
:   defaultScheduledTimeRange(defaultScheduledTimeRange),
    schedulableTimeRange(schedulableTimeRange),
    id(id),
    policy(policy),
    dependencies(dependencies),
    tags(tags)
{
        return;
};

bool Job::isRigid() {
    return defaultScheduledTimeRange == schedulableTimeRange;
}