#ifndef ELASTISCHED_CONSTANTS_H
#define ELASTISCHED_CONSTANTS_H

#include <stdint.h>
#include <stddef.h>

typedef uint32_t sec_t;

static const sec_t MINUTE = (sec_t)60;
static const sec_t HOUR_TO_MINUTES = (sec_t)60;
static const sec_t DAY_TO_HOURS = (sec_t)24;
static const sec_t WEEK_TO_DAYS = (sec_t)7;

static const sec_t HOUR = ((sec_t)60 * MINUTE);
static const sec_t DAY = ((sec_t)24 * HOUR);
static const sec_t WEEK = ((sec_t)7 * DAY);

static const sec_t AFTERNOON_START = 17;

static const double FRIDAY_HOURLY_COST_FACTOR = 2.0;
static const double SATURDAY_HOURLY_COST_FACTOR = 3.0;

static const double EXP_DOWNFACTOR = 0.1;
static const double HOURLY_COST_FACTOR = 1.0;

static const double ILLEGAL_SCHEDULE_COST = 1e12;

static const double EPSILON = 1e-5;
static const unsigned int DEFAULT_RNG_SEED = 1337;

static const size_t INITIAL_TAGCONTAINER_CAPACITY = 8;
static const size_t INITIAL_CONTAINER_CAPACITY = 256;
static const size_t INITIAL_MAP_CAPACITY = 32;

static const size_t ELASTISCHED_INTERNAL_DLL_SZ = 3 * sizeof(size_t);

#endif
