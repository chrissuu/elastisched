#include "job.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

bool job_is_rigid(Job *job) {
    if (!job) return false;
    return job->duration == interval_length(&job->schedulable_time_range);
}

char* job_to_string(Job* job) {
    if (!job) return NULL;
    const char* name = job->id.name ? job->id.name : "(null)";

    int needed = snprintf(NULL, 0,
        "Job{id=%s, duration=%" PRIu32 ", schedulable=[%" PRIu32 ",%" PRIu32 "], scheduled=[%" PRIu32 ",%" PRIu32 "]}",
        name,
        (uint32_t)job->duration,
        (uint32_t)job->schedulable_time_range.low,
        (uint32_t)job->schedulable_time_range.high,
        (uint32_t)job->scheduled_time_range.low,
        (uint32_t)job->scheduled_time_range.high);

    if (needed < 0) return NULL;
    size_t size = (size_t)needed + 1;
    char* buf = malloc(size);
    if (!buf) return NULL;
    snprintf(buf, size,
        "Job{id=%s, duration=%" PRIu32 ", schedulable=[%" PRIu32 ",%" PRIu32 "], scheduled=[%" PRIu32 ",%" PRIu32 "]}",
        name,
        (uint32_t)job->duration,
        (uint32_t)job->schedulable_time_range.low,
        (uint32_t)job->schedulable_time_range.high,
        (uint32_t)job->scheduled_time_range.low,
        (uint32_t)job->scheduled_time_range.high);
    return buf;
}
