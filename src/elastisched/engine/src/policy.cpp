#include <cstdint>
#include "policy.hpp"

Policy::Policy(uint8_t max_splits,
               time_t min_split_duration,
               uint8_t scheduling_policies,
               bool round_to_granularity)
        : max_splits(max_splits),
          min_split_duration(min_split_duration),
          round_to_granularity(round_to_granularity),
          scheduling_policies(scheduling_policies) {}

uint8_t Policy::getMaxSplits() const { return max_splits; }
time_t Policy::getMinSplitDuration() const { return min_split_duration; }
bool Policy::getRoundToGranularity() const { return round_to_granularity; }
uint8_t Policy::getSchedulingPolicies() const { return scheduling_policies; }

bool Policy::isSplittable() const {
    return scheduling_policies & static_cast<char>(1);
}

bool Policy::isOverlappable() const {
    return (scheduling_policies & static_cast<char>(2)) >> 1;
}

bool Policy::isInvisible() const {
    return (scheduling_policies & static_cast<char>(3)) >> 2;
}
