#ifndef JOB_H
#define JOB_H

#include "policy.h"
#include "timerange.h"
#include "tag.h"
#include "string.h"

using ID = char*;

typedef struct {
    TimeRange default_scheduled_timerange;
    TimeRange schedulable_timerange;
    ID id;
    ID* dependencies;
    Tag* tags;
    Policy policy;
} Job;

#endif // JOB_H