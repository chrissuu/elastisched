#ifndef POLICY_HPP
#define POLICY_HPP

#include <cstdint>
#include "constants.hpp"

/**
 * Policy
 * 
 * Policy defines how jobs can be scheduled.
 * 
 * scheduling_policies is an overloaded integer
 * where the two least significant bits represent:
 *      -> is_splittable (bit 0)
 *      -> is_overlappable (bit 1)
 */
class Policy {
private:
    uint8_t max_splits;
    time_t min_split_duration;
    uint8_t scheduling_policies;  // Bitfield: bit 0 = is_splittable, bit 1 = is_overlappable, bit 2 = is_invisible

public:
    Policy(uint8_t max_splits, time_t min_split_duration, uint8_t scheduling_policies);

    uint8_t getMaxSplits() const;
    time_t getMinSplitDuration() const;
    uint8_t getSchedulingPolicies() const;

    bool isSplittable() const;
    bool isOverlappable() const;
    bool isInvisible() const;
};

#endif // POLICY_HPP
