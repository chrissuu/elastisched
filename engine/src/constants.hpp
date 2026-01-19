#ifndef CONSTANTS
#define CONSTANTS

#include <cstdint>
#include <cstdlib>
#include <string>

#include "types.hpp"

namespace constants {
    constexpr sec_t DAY = (uint64_t)24 * (uint64_t)60 * (uint64_t)60;
    constexpr double SPLIT_COST_FACTOR = 10.0f;
    constexpr double ILLEGAL_SCHEDULE_COST = 1e12f;
    constexpr double EPSILON = 1e-5f;
    constexpr uint32_t DEFAULT_RNG_SEED = 1337;

    inline uint32_t RNG_SEED() {
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