#include <cstdint>
#include "policy.hpp"

namespace {
const uint8_t kPolicySplittable = 1 << 0;
const uint8_t kPolicyOverlappable = 1 << 1;
const uint8_t kPolicyInvisible = 1 << 2;
const uint8_t kPolicyRoundToGranularity = 1 << 3;
}  // namespace

Policy::Policy(uint8_t max_splits,
               sec_t min_split_duration,
               bool is_splittable,
               bool is_overlappable,
               bool is_invisible,
               bool round_to_granularity)
        : max_splits(max_splits),
          min_split_duration(min_split_duration),
          scheduling_policies(0) {
    if (is_splittable) {
        scheduling_policies |= kPolicySplittable;
    }
    if (is_overlappable) {
        scheduling_policies |= kPolicyOverlappable;
    }
    if (is_invisible) {
        scheduling_policies |= kPolicyInvisible;
    }
    if (round_to_granularity) {
        scheduling_policies |= kPolicyRoundToGranularity;
    }
}

uint8_t Policy::get_max_splits() const { return max_splits; }
sec_t Policy::get_min_split_duration() const { return min_split_duration; }
bool Policy::get_round_to_granularity() const {
    return (scheduling_policies & kPolicyRoundToGranularity) != 0;
}
uint8_t Policy::get_scheduling_policies() const { return scheduling_policies; }

bool Policy::is_splittable() const {
    return (scheduling_policies & kPolicySplittable) != 0;
}

bool Policy::is_overlappable() const {
    return (scheduling_policies & kPolicyOverlappable) != 0;
}

bool Policy::is_invisible() const {
    return (scheduling_policies & kPolicyInvisible) != 0;
}
