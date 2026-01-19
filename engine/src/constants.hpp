#ifndef CONSTANTS
#define CONSTANTS

#include <cstdlib>
#include <string>

#include "Interval.hpp"
#include "Tag.hpp"

#define time_t uint64_t
#define TimeRange Interval<time_t>
#define ID std::string


namespace constants {
    constexpr time_t minute = (uint64_t)60;
    constexpr time_t hour_to_minutes = (uint64_t)60;
    constexpr time_t day_to_hours = (uint64_t)24;
    constexpr time_t week_to_days = (uint64_t)7;

    constexpr time_t hour = ((uint64_t)60 * minute);
    constexpr time_t day = ((uint64_t)24 * hour);
    constexpr time_t week = ((uint64_t)7 * day);

    constexpr time_t afternoon_start = 17;

    constexpr double friday_hourly_cost_factor = 2.0f;
    constexpr double saturday_hourly_cost_factor = 3.0f;

    constexpr double exp_downfactor = 0.1f;
    constexpr double hourly_cost_factor = 1.0f;
    constexpr double split_cost_factor = 10.0f;

    const Tag work_tag = Tag("ELASTISCHED_WORK_TYPE");
    constexpr double illegal_schedule_cost = 1e12f;

    constexpr double epsilon = 1e-5f;
    constexpr uint32_t default_rng_seed = 1337;

    inline uint32_t rng_seed() {
        const char* value = std::getenv("ELASTISCHED_RNG_SEED");
        if (!value || !*value) {
            return default_rng_seed;
        }
        char* end = nullptr;
        unsigned long parsed = std::strtoul(value, &end, 10);
        if (end == value || *end != '\0') {
            return default_rng_seed;
        }
        return static_cast<uint32_t>(parsed);
    }
}
#endif
