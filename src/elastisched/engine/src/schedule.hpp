#ifndef SCHEDULE_H
#define SCHEDULE_H

#include "policy.hpp"
#include "tag.hpp"
#include "job.hpp"

#include "constants.hpp"

struct Schedule {
    std::vector<Job> scheduledJobs;
};

#endif