#ifndef CONSTANTS
#define CONSTANTS

#include <stdint.h>
#include <stdio.h>

typedef uint32_t sec_t;

const sec_t MINUTE = (sec_t)60;
const sec_t HOUR_TO_MINUTES = (sec_t)60;
const sec_t DAY_TO_HOURS = (sec_t)24;
const sec_t WEEK_TO_DAYS = (sec_t)7;

const sec_t HOUR = ((sec_t)60 * MINUTE);
const sec_t DAY = ((sec_t)24 * HOUR);
const sec_t WEEK = ((sec_t)7 * DAY);

const sec_t AFTERNOON_START = 17;

const double FRIDAY_HOURLY_COST_FACTOR = 2.0f;
const double SATURDAY_HOURLY_COST_FACTOR = 3.0f;

const double EXP_DOWNFACTOR = 0.1f;
const double HOURLY_COST_FACTOR = 1.0f;

const double ILLEGAL_SCHEDULE_COST = 1e12f;

const double EPSILON = 1e-5f;
const unsigned int DEFAULT_RNG_SEED = 1337;

const size_t INITIAL_TAGCONTAINER_CAPACITY = 8;
const size_t INITIAL_CONTAINER_CAPACITY = 256;
const size_t INITIAL_MAP_CAPACITY = 32;

const size_t ELASTISCHED_INTERNAL_DLL_SZ = 3 * sizeof(size_t);

#endif
