#ifndef INTERVAL_H
#define INTERVAL_H

#include <stdbool.h>
#include <stdlib.h>
#include "constants.h"

typedef struct Interval {
    sec_t low;
    sec_t high;
} Interval;

typedef enum { 
    RED, 
    BLACK } 
Color;

typedef struct Node Node;

struct Node {
    Node* left;
    Node* right;
    Node* parent;
    Interval* interval;
    void* value;
    sec_t max;
    Color color;
};

typedef struct IntervalMap {
    Node* root;
} IntervalMap;

bool interval_eq(const Interval* U, const Interval* V);
bool interval_overlaps(const Interval* U, const Interval* V);
bool interval_contains(const Interval* U, const Interval* V);
sec_t interval_length(const Interval* interval);
bool interval_is_valid(const Interval* interval);

Node* mk_leaf_node(Node* parent, Interval* interval, 
    void* value, sec_t max, Color color);
Node* mk_node(Node* left, Node* right, Node* parent,
    Interval* interval, void* value, sec_t max, Color color);

IntervalMap* mk_intmap(Node* root);
void intmap_insert(IntervalMap* map, Interval* key, void* value);
void intmap_free(IntervalMap* map);
#endif
