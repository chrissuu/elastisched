#ifndef JOB_H
#define JOB_H

#include "constants.h"
#include "utils.h"
#include "interval.h"
#include "policy.h"
#include "tag.h"

typedef struct Job {
    sec_t duration;
    Interval schedulable_time_range;
    Interval scheduled_time_range;
    ID id;
    Policy policy;
    DependencyContainer* dependency_set;
    TagContainer* tag_set;
} Job;

typedef struct JobVec {
    Job* data;
    size_t size;
    size_t capacity;
} JobVec;

bool job_is_rigid(Job* job);
char* job_to_string(Job* job);

#endif