#include "policy.h"

bool policy_is_splittable(Policy* policy) {
    return policy && (policy->scheduling_policies & 1u) != 0;
}

bool policy_is_overlappable(Policy* policy) {
    return policy && ((policy->scheduling_policies & 2u) >> 1) != 0;
}

bool policy_is_invisible(Policy* policy) {
    return policy && ((policy->scheduling_policies & 4u) >> 2) != 0;
}
