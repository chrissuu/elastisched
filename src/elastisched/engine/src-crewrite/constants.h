#ifndef CONSTANTS
#define CONSTANTS

#include <stdint.h>

#include "tag.h"

typedef uint32_t time_t;

const time_t MINUTE = (time_t)60;
const time_t HOUR_TO_MINUTES = (time_t)60;
const time_t DAY_TO_HOURS = (time_t)24;
const time_t WEEK_TO_DAYS = (time_t)7;

const time_t HOUR = ((time_t)60 * MINUTE);
const time_t DAY = ((time_t)24 * HOUR);
const time_t WEEK = ((time_t)7 * DAY);

const time_t AFTERNOON_START = 17;

const double FRIDAY_HOURLY_COST_FACTOR = 2.0f;
const double SATURDAY_HOURLY_COST_FACTOR = 3.0f;

const double EXP_DOWNFACTOR = 0.1f;
const double HOURLY_COST_FACTOR = 1.0f;

const struct Tag WORK_TAG = {.name="ELASTISCHED_WORK_TYPE\0"};
const double ILLEGAL_SCHEDULE_COST = 1e12f;

const double EPSILON = 1e-5f;
const unsigned int DEFAULT_RNG_SEED = 1337;
#endif
