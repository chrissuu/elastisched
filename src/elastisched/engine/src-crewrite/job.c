#include "job.h"

bool job_is_rigid(Job *job) {
    return job->duration == interval_length(&job->schedulable_time_range);
}