#ifndef INTERVAL_H
#define INTERVAL_H

#include <stdbool.h>
#include "constants.h"

typedef struct Interval {
    time_t low;
    time_t high;
} Interval;

typedef enum { 
    RED, 
    BLACK } 
Color;

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
    void* root;
} IntervalMap;

bool interval_eq(Interval* U, Interval* V);
bool interval_overlaps(Interval* U, Interval* V);
bool interval_contains(Interval* U, Interval* V);
time_t interval_length(Interval* interval);
bool interval_is_valid(Interval* interval);

Node* mk_leaf_node(Node* parent, Interval* interval, 
    void* value, time_t max, Color color);
Node* mk_node(Node* left, Node* right, Node* parent,
    Interval* interval, void* value, time_t max, Color color);

IntervalMap* mk_intmap(Node* root);
void intmap_insert(IntervalMap* map, Interval* key, void* value);
void intmap_free(IntervalMap* map);
#endif