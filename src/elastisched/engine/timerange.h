#ifndef TIMERANGE_H
#define TIMERANGE_H

#include <stdint.h>

#define time_t uint64_t
typedef struct {
    time_t start; // milliseconds since an epoch
    time_t end;
} TimeRange;

bool troverlaps(TimeRange tr1, TimeRange tr2) {
    return tr1.start < tr2.end && tr2.start < tr1.end;
}


bool trcontains(TimeRange tr1, TimeRange tr2) {
    return tr1.start <= tr2.start && tr2.end <= tr1.end;
}


bool trequals(TimeRange tr1, TimeRange tr2) {
    return tr1.start == tr2.start && tr1.end == tr2.end;
}

#endif
