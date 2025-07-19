#include <cstdint>
#include "policy.hpp"

Policy::Policy(uint8_t max_splits, double min_split_duration, uint8_t scheduling_policies)
        : max_splits(max_splits),
          min_split_duration(min_split_duration),
          scheduling_policies(scheduling_policies) {}

uint8_t Policy::getMaxSplits() const { return max_splits; }
double Policy::getMinSplitDuration() const { return min_split_duration; }
uint8_t Policy::getSchedulingPolicies() const { return scheduling_policies; }

bool Policy::isSplittable() const {
    return scheduling_policies & static_cast<char>(1);
}

bool Policy::isOverlappable() const {
    return (scheduling_policies & static_cast<char>(2)) >> 1;
}