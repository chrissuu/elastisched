#include "policy.h"

/**
 * Policy
 * 
 * Policy defines how jobs can be scheduled.
 * 
 * scheduling_policies is an overloaded integer
 * where the two least significant bits represent
 * the following scheduling policies:
 *      -> is_splittable (bit 0)
 *      -> is_overlappable (bit 1)
 */
typedef struct {
    uint8_t max_splits;
    double min_split_duration;
    char scheduling_policies;
} Policy;

uint8_t is_splittable(const Policy *P) {
    return P->scheduling_policies & (uint8_t)1;
}

uint8_t is_overlappable(const Policy *P) {
    return (P->scheduling_policies & (uint8_t)2) >> (uint8_t)1;
}