#ifndef SCHEDULE_H
#define SCHEDULE_H

#include "job.h"
#include "utils.h"
#include "vec.h"

typedef struct Schedule {
    JobVec* scheduled_jobs;
} Schedule;

void schedule_add_job(Schedule* schedule, const Job* job);
void schedule_clear(Schedule* schedule);

#endif