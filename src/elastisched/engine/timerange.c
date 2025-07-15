#include "timerange.h"
#include "stdbool.h"

/**
 * TimeRange
 * 
 * TimeRange defines a range of times.
 */
typedef struct {
    time_t start;
    time_t end;
} TimeRange;


/**
 * Checks if tr1 overlaps with tr2.
 * 
 * @param tr1 TimeRange A
 * @param tr2 TimeRange B
 * @return True if A overlaps with B
 */
bool overlaps(TimeRange tr1, TimeRange tr2);


/**
 * Checks if tr1 contains tr2.
 * 
 * @param tr1 TimeRange A
 * @param tr2 TimeRange B
 * @return True if A contains B
 */
bool contains(TimeRange tr1, TimeRange tr2);


/**
 * Checks if tr1 equals tr2.
 * 
 * @param tr1 TimeRange A
 * @param tr2 TimeRange B
 * @return True if A == B
 */
bool equals(TimeRange tr1, TimeRange tr2);
