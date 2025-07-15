#ifndef SCHEDULE_H
#define SCHEDULE_H

#include "policy.h"
#include "tag.h"
#include "timerange.h"

typedef struct {
    Policy policy;
    Tag* tags;
    TimeRange scheduled_timerange;
} ScheduledJob;

typedef struct {
    ScheduledJob* scheduled_jobs;
} Schedule;
#endif
