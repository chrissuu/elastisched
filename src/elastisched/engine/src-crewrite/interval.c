#include "interval.h"

static sec_t node_max(Node* node) {
    return node ? node->max : 0;
}

static void update_max(Node* node) {
    if (!node || !node->interval) return;
    sec_t left_max = node_max(node->left);
    sec_t right_max = node_max(node->right);
    sec_t interval_high = node->interval->high;

    sec_t max = interval_high;
    if (left_max > max) max = left_max;
    if (right_max > max) max = right_max;
    node->max = max;
}

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
    if (!map || !key) return;
    if (!interval_is_valid(key)) return;

    if (!map->root) {
        map->root = mk_leaf_node(NULL, key, value, key->high, BLACK);
        return;
    }

    Node* curr = map->root;
    Node* parent = NULL;
    while (curr) {
        parent = curr;
        if (key->low < curr->interval->low) {
            curr = curr->left;
        } else {
            curr = curr->right;
        }
    }

    Node* node = mk_leaf_node(parent, key, value, key->high, BLACK);
    if (!node) return;

    if (key->low < parent->interval->low) {
        parent->left = node;
    } else {
        parent->right = node;
    }

    for (Node* n = parent; n; n = n->parent) {
        update_max(n);
    }
}

void intmap_delete(IntervalMap* map) {
    return;
}

void intmap_free(IntervalMap* map) {
    if (!map) return;
    if (!map->root) {
        free(map);
        return;
    }

    size_t capacity = 32;
    size_t top = 0;
    Node** stack = malloc(capacity * sizeof(Node*));
    if (!stack) {
        free(map);
        return;
    }
    stack[top++] = map->root;

    while (top > 0) {
        Node* node = stack[--top];
        if (node->left) {
            if (top == capacity) {
                capacity *= 2;
                Node** new_stack = realloc(stack, capacity * sizeof(Node*));
                if (!new_stack) break;
                stack = new_stack;
            }
            stack[top++] = node->left;
        }
        if (node->right) {
            if (top == capacity) {
                capacity *= 2;
                Node** new_stack = realloc(stack, capacity * sizeof(Node*));
                if (!new_stack) break;
                stack = new_stack;
            }
            stack[top++] = node->right;
        }
        free(node);
    }

    free(stack);
    free(map);
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
