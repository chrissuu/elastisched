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
    bool round_to_granularity;
    uint8_t scheduling_policies;  // Bitfield: bit 0 = is_splittable, bit 1 = is_overlappable, bit 2 = is_invisible

public:
    Policy(uint8_t max_splits = 0,
           time_t min_split_duration = 0,
           uint8_t scheduling_policies = 0,
           bool round_to_granularity = false);

    uint8_t get_max_splits() const;
    time_t get_min_split_duration() const;
    bool get_round_to_granularity() const;
    uint8_t get_scheduling_policies() const;

    bool is_splittable() const;
    bool is_overlappable() const;
    bool is_invisible() const;
};

#endif // POLICY_HPP
