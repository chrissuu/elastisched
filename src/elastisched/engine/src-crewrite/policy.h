#ifndef POLICY_H
#define POLICY_H

#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include "constants.h"

/**
 * @brief Scheduling policy struct, which
 * defines the configurations for how a job
 * may be scheduled. Each job may have its own policy. 
 */
typedef struct Policy {
    uint8_t max_splits;          /// how many times a splittable job may be split
    time_t min_split_duration;   /// minimum duration that a splittable job can be split into
    uint8_t scheduling_policies; /// overloaded bitfield: bit 0 = is_splittable, bit 1 = is_overlappable, bit 2 = is_invisible
} Policy;

bool policy_is_splittable(Policy* policy);
bool policy_is_overlappable(Policy* policy);
bool policy_is_invisible(Policy* policy);

#endif
