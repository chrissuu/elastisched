#ifndef INTERVAL_H
#define INTERVAL_H

#include <stdbool.h>
#include "constants.h"

typedef struct Interval {
    time_t low;
    time_t high;
} Interval;

enum Color { RED, BLACK };

typedef struct Node {
    Node* left;
    Node* right;
    Node* parent;
    Interval* interval;
    void* value;
    time_t max;
    Color color;
} Node;

typedef struct IntervalMap {
    Node* root;
} IntervalMap;

time_t interval_get_low(Interval* interval);
time_t interval_get_high(Interval* interval);
bool interval_eq(Interval* U, Interval* V);
bool interval_overlaps(Interval* U, Interval* V);
bool interval_contains(Interval* U, Interval* V);
time_t interval_length(Interval* interval);
bool is_valid_interval(Interval* interval);

IntervalMap* mk_interval_map();
void interval_map_insert(IntervalMap*, Interval* key, void* value);

void free_interval_map(IntervalMap* map);
#endif

