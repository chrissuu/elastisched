// policy.hpp
#ifndef POLICY_HPP
#define POLICY_HPP

#include <cstdint>

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
    double min_split_duration;
    uint8_t scheduling_policies;  // Bitfield: bit 0 = is_splittable, bit 1 = is_overlappable

public:
    // Constructor
    Policy(uint8_t max_splits, double min_split_duration, uint8_t scheduling_policies);

    // Accessors
    uint8_t getMaxSplits() const;
    double getMinSplitDuration() const;
    uint8_t getSchedulingPolicies() const;

    // Bitwise policy checks
    bool isSplittable() const;
    bool isOverlappable() const;
};

#endif // POLICY_HPP
