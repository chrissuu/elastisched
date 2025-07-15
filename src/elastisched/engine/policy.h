#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

/**
 * Policy
 * 
 * Policy defines how jobs can be scheduled.
 * 
 * scheduling_policies is an overloaded integer
 * where the two least significant bits represent:
 *      bit 0 -> is_splittable
 *      bit 1 -> is_overlappable
 */
typedef struct {
    uint8_t max_splits;
    double min_split_duration;
    char scheduling_policies;
} Policy;

/**
 * Checks if P defines a splittable policy
 * 
 * @param P
 * @return 1 if P defines a splittable policy, 0 otherwise
 */
uint8_t is_splittable(const Policy *P);


/**
 * Checks if P defines an overlappable policy
 * 
 * @param P
 * @return 1 if P defines a overlappable policy, 0 otherwise
 */
uint8_t is_overlappable(const Policy *P);


#endif // UTILS_H
