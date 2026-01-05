#include "schedule.h"

void schedule_add_job(Schedule* schedule, const Job* job) {
    vec_pushback(schedule->scheduled_jobs, *job);
    return;
}

void schedule_clear(Schedule* schedule) {
    vec_clear(schedule->scheduled_jobs);
}