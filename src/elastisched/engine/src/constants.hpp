#ifndef CONSTANTS
#define CONSTANTS

#include <string>

#include "utils/Interval.hpp"

#define time_t uint64_t
#define TimeRange Interval<time_t>
#define ID std::string


namespace constants {
    constexpr time_t MINUTE_TO_SECONDS = (uint64_t)60;
    constexpr time_t HOUR_TO_MINUTES = (uint64_t)60;
    constexpr time_t DAY_TO_HOURS = (uint64_t)24;
    constexpr time_t WEEK_TO_DAYS = (uint64_t)7;
    constexpr time_t DAY_TO_SECONDS = (MINUTE_TO_SECONDS * HOUR_TO_MINUTES * DAY_TO_HOURS);
    constexpr time_t WEEK_TO_SECONDS = ((uint64_t)7 * DAY_TO_SECONDS);

    constexpr double FRIDAY_HOURLY_COST_FACTOR = 2.0f;
    constexpr double SATURDAY_HOURLY_COST_FACTOR = 3.0f;
}
#endif