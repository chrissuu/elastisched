#ifndef CONSTANTS
#define CONSTANTS

#include <cstdlib>
#include <string>

#include "utils/Interval.hpp"
#include "tag.hpp"

#define time_t uint64_t
#define TimeRange Interval<time_t>
#define ID std::string


namespace constants {
    constexpr time_t MINUTE = (uint64_t)60;
    constexpr time_t HOUR_TO_MINUTES = (uint64_t)60;
    constexpr time_t DAY_TO_HOURS = (uint64_t)24;
    constexpr time_t WEEK_TO_DAYS = (uint64_t)7;

    constexpr time_t HOUR = ((uint64_t)60 * MINUTE);
    constexpr time_t DAY = ((uint64_t)24 * HOUR);
    constexpr time_t WEEK = ((uint64_t)7 * DAY);

    constexpr time_t AFTERNOON_START = 17;

    constexpr double FRIDAY_HOURLY_COST_FACTOR = 2.0f;
    constexpr double SATURDAY_HOURLY_COST_FACTOR = 3.0f;

    constexpr double EXP_DOWNFACTOR = 0.1f;
    constexpr double HOURLY_COST_FACTOR = 1.0f;
    constexpr double SPLIT_COST_FACTOR = 10.0f;

    const Tag WORK_TAG = Tag("ELASTISCHED_WORK_TYPE");
    constexpr double ILLEGAL_SCHEDULE_COST = 1e12f;

    constexpr double EPSILON = 1e-5f;
    constexpr uint32_t DEFAULT_RNG_SEED = 1337;

    inline uint32_t rng_seed() {
        const char* value = std::getenv("ELASTISCHED_RNG_SEED");
        if (!value || !*value) {
            return DEFAULT_RNG_SEED;
        }
        char* end = nullptr;
        unsigned long parsed = std::strtoul(value, &end, 10);
        if (end == value || *end != '\0') {
            return DEFAULT_RNG_SEED;
        }
        return static_cast<uint32_t>(parsed);
    }
}
#endif
