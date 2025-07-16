#include <cstdint>
#include "policy.hpp"

class Policy {
private:
    uint8_t max_splits;
    double min_split_duration;
    char scheduling_policies;  // Bitfield: bit 0 = is_splittable, bit 1 = is_overlappable

public:
    Policy(uint8_t max_splits, double min_split_duration, char scheduling_policies)
        : max_splits(max_splits),
          min_split_duration(min_split_duration),
          scheduling_policies(scheduling_policies) {}

    uint8_t getMaxSplits() const { return max_splits; }
    double getMinSplitDuration() const { return min_split_duration; }
    char getSchedulingPolicies() const { return scheduling_policies; }

    bool isSplittable() const {
        return scheduling_policies & static_cast<char>(1);
    }

    bool isOverlappable() const {
        return (scheduling_policies & static_cast<char>(2)) >> 1;
    }
};
