#include "interval.h"

bool interval_eq(const Interval* U, const Interval* V) {
    return (U->low == V->low) && (U->high == V->high);
}

bool interval_overlaps(const Interval* U, const Interval* V) {
    return !(U->high < V->low || V->high < U->low);
}

bool interval_contains(const Interval* U, const Interval* V) {
    return (U->low <= V->low && V->high <= U->high);
}

sec_t interval_length(const Interval* interval) {
    return interval->high - interval->low;
}

bool interval_is_valid(const Interval* interval) {
    return interval->high >= interval->low;
}

Node* mk_leaf_node(Node* parent, Interval* interval,
    void* value, sec_t max, Color color
) {
    Node* node = malloc(sizeof(Node));
    if (!node) return NULL;

    node->parent = parent;
    node->interval = interval;
    node->value = value;
    node->max = max;
    node->color = color;

    node->left = NULL;
    node->right = NULL;

    return node;
}

Node* mk_node(Node* left, Node* right, Node* parent,
    Interval* interval, void* value, sec_t max, Color color
) {
    Node* node = malloc(sizeof(Node));
    if (!node) return NULL;

    node->left = left;
    node->right = right;
    node->parent = parent;
    node->interval = interval;
    node->value = value;
    node->max = max;
    node->color = color;

    return node;
}

IntervalMap* mk_intmap(Node* root) {
    IntervalMap* map = malloc(sizeof(IntervalMap));
    if (!map) return NULL;

    map->root = root;
    return map;
}

void intmap_insert(IntervalMap* map, Interval* key, void* value) {
    return;
}

void intmap_delete(IntervalMap* map) {
    return;
}

void intmap_free(IntervalMap* map) {
    return;
}

void intmap_left_rotate(IntervalMap* map, Node* x) {
    Node* y = x->right;
    x->right = y->left;
    if (y->left) y->left->parent = x;

    y->parent = x->parent;
    if (!x->parent) {
        map->root = y;
    } else if (x == x->parent->left) {
        x->parent->left = y;
    } else {
        x->parent->right = y;
    }
    y->left = x;
    x->parent = y;
}

void intmap_right_rotate(IntervalMap* map, Node* y) {
    Node* x = y->left;

    y->left = x->right;
    if (x->right) x->right->parent = y;

    x->parent = y->parent;
    if (!y->parent) {
        map->root = x;
    } else if (y == y->parent->left) {
        y->parent->left = x;
    } else {
        y->parent->right = x;
    }

    x->right = y;
    y->parent = x;
}

