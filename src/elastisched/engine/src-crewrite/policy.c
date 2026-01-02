#include "policy.h"

uint8_t get_max_splits(Policy* policy) { return policy->max_splits; }
time_t get_min_split_duration(Policy* policy) { return policy->min_split_duration; }
uint8_t get_scheduling_policies(Policy* policy) { return policy->scheduling_policies; }

bool is_splittable(Policy* policy) { return (policy->scheduling_policies & 1u) != 0; }
bool is_overlappable(Policy* policy) { return ((policy->scheduling_policies & 2u) >> 1) != 0; }
bool is_invisible(Policy* policy) { return ((policy->scheduling_policies & 4u) >> 2) != 0; }