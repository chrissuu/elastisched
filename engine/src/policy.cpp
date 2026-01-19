#include <cstdint>
#include "Policy.hpp"

Policy::Policy(uint8_t max_splits,
               time_t min_split_duration,
               uint8_t scheduling_policies,
               bool round_to_granularity)
        : max_splits(max_splits),
          min_split_duration(min_split_duration),
          round_to_granularity(round_to_granularity),
          scheduling_policies(scheduling_policies) {}

uint8_t Policy::get_max_splits() const { return max_splits; }
time_t Policy::get_min_split_duration() const { return min_split_duration; }
bool Policy::get_round_to_granularity() const { return round_to_granularity; }
uint8_t Policy::get_scheduling_policies() const { return scheduling_policies; }

bool Policy::is_splittable() const {
    return scheduling_policies & static_cast<char>(1);
}

bool Policy::is_overlappable() const {
    return (scheduling_policies & static_cast<char>(2)) >> 1;
}

bool Policy::is_invisible() const {
    return (scheduling_policies & static_cast<char>(3)) >> 2;
}
