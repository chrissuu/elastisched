#ifndef SCHEDULE_H
#define SCHEDULE_H

#include "policy.hpp"
#include "tag.hpp"
#include "job.hpp"

#include "constants.hpp"

#include <vector>
#include <set>

struct ScheduledJob {
    std::set<Tag> tags;
    TimeRange scheduledTimeRange;
    ID id;
};

struct Schedule {
    std::vector<ScheduledJob> scheduledJobs;
};

#endif
