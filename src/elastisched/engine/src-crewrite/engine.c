#include "engine.h"

interval_t* mk_interval(sec_t low, sec_t high) {
    interval_t* interval = malloc(sizeof(interval_t));
    if (!interval) return NULL;

    interval->low = low; interval->high = high;
    return interval;
}

void interval_free(interval_t* interval) { free(interval); }

bool interval_eq(const interval_t* U, const interval_t* V) {
    return (U->low == V->low) && (U->high == V->high);
}

bool interval_overlaps(const interval_t* U, const interval_t* V) {
    return !(U->high < V->low || V->high < U->low);
}

bool interval_contains(const interval_t* U, const interval_t* V) {
    return (U->low <= V->low && V->high <= U->high);
}

sec_t interval_length(const interval_t* interval) {
    return interval->high - interval->low;
}

bool interval_is_valid(const interval_t* interval) {
    return interval->high >= interval->low;
}

/**
 * @brief Scheduling policy struct, which
 * defines the configurations for how a job
 * may be scheduled. Each job may have its own policy. 
 */
typedef struct policy {
    uint8_t max_splits;          /// how many times a splittable job may be split
    sec_t min_split_duration;   /// minimum duration that a splittable job can be split into
    uint8_t scheduling_policies; /// overloaded bitfield: bit 0 = is_splittable, bit 1 = is_overlappable, bit 2 = is_invisible
} policy_t;

policy_t *mk_policy(bool is_splittable, bool is_overlappable, bool is_invisible,
                    uint8_t max_splits, sec_t min_split_duration) {
    policy_t* policy = malloc(sizeof(policy_t));
    if (!policy) return NULL;

    policy->max_splits = max_splits;
    policy->min_split_duration = min_split_duration;

    policy->scheduling_policies = 0;
    if (is_splittable) {
        policy->scheduling_policies |= 1u;
    }
    if (is_overlappable) {
        policy->scheduling_policies |= 2u;
    }
    if (is_invisible) {
        policy->scheduling_policies |= 4u;
    }

    return policy;
};

uint8_t policy_max_splits(policy_t *policy) { return policy->max_splits; };

sec_t min_split_duration(policy_t *policy) { return policy->min_split_duration; };

bool policy_is_splittable(policy_t* policy) {
    return policy && (policy->scheduling_policies & 1u) != 0; }

bool policy_is_overlappable(policy_t* policy) {
    return policy && ((policy->scheduling_policies & 2u) >> 1) != 0; }

bool policy_is_invisible(policy_t* policy) {
    return policy && ((policy->scheduling_policies & 4u) >> 2) != 0; }


bool job_is_rigid(job_t *job) {
    if (!job) return false;
    return job->duration == interval_length(&job->schedulable_tr);
}

typedef enum { 
    RED, 
    BLACK } 
Color;

typedef struct node node_t;

struct node {
    node_t* left;
    node_t* right;
    node_t* parent;
    interval_t* interval;
    void* value;
    sec_t max;
    Color color;
};

typedef struct interval_tMap {
    node_t* root;
} interval_tMap;

static sec_t node_max(node_t* node) {
    return node ? node->max : 0;
}

static void update_max(node_t* node) {
    if (!node || !node->interval) return;
    sec_t left_max = node_max(node->left);
    sec_t right_max = node_max(node->right);
    sec_t interval_high = node->interval->high;

    sec_t max = interval_high;
    if (left_max > max) max = left_max;
    if (right_max > max) max = right_max;
    node->max = max;
}

node_t* mk_leaf_node(node_t* parent, interval_t* interval,
    void* value, sec_t max, Color color
) {
    node_t* node = malloc(sizeof(node_t));
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

node_t* mk_node(node_t* left, node_t* right, node_t* parent,
    interval_t* interval, void* value, sec_t max, Color color
) {
    node_t* node = malloc(sizeof(node_t));
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

interval_tMap* mk_intmap(node_t* root) {
    interval_tMap* map = malloc(sizeof(interval_tMap));
    if (!map) return NULL;

    map->root = root;
    return map;
}

void intmap_insert(interval_tMap* map, interval_t* key, void* value) {
    if (!map || !key) return;
    if (!interval_is_valid(key)) return;

    if (!map->root) {
        map->root = mk_leaf_node(NULL, key, value, key->high, BLACK);
        return;
    }

    node_t* curr = map->root;
    node_t* parent = NULL;
    while (curr) {
        parent = curr;
        if (key->low < curr->interval->low) {
            curr = curr->left;
        } else {
            curr = curr->right;
        }
    }

    node_t* node = mk_leaf_node(parent, key, value, key->high, BLACK);
    if (!node) return;

    if (key->low < parent->interval->low) {
        parent->left = node;
    } else {
        parent->right = node;
    }

    for (node_t* n = parent; n; n = n->parent) {
        update_max(n);
    }
}

void intmap_delete(interval_tMap* map) {
    return;
}

void intmap_free(interval_tMap* map) {
    if (!map) return;
    if (!map->root) {
        free(map);
        return;
    }

    size_t capacity = 32;
    size_t top = 0;
    node_t** stack = malloc(capacity * sizeof(node_t*));
    if (!stack) {
        free(map);
        return;
    }
    stack[top++] = map->root;

    while (top > 0) {
        node_t* node = stack[--top];
        if (node->left) {
            if (top == capacity) {
                capacity *= 2;
                node_t** new_stack = realloc(stack, capacity * sizeof(node_t*));
                if (!new_stack) break;
                stack = new_stack;
            }
            stack[top++] = node->left;
        }
        if (node->right) {
            if (top == capacity) {
                capacity *= 2;
                node_t** new_stack = realloc(stack, capacity * sizeof(node_t*));
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

void intmap_left_rotate(interval_tMap* map, node_t* x) {
    node_t* y = x->right;
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

void intmap_right_rotate(interval_tMap* map, node_t* y) {
    node_t* x = y->left;

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

/**
 * @brief 
 *
 * TagSet in practice does not get very large nor updated
 * often so we opt for a simple vector-based sorted set.
 * 
 * Two tags are equivalent if they share the same name,
 * not necessarily the same description.
 * 
 * This allows for O(logn) search time and membership
 * checking, O(n) set difference and O(n) time to add
 * to the set.
 */

struct tag {
    char* name;
    char* description;
};

bool tag_eq(const tag_t* U, const tag_t* V) {
    return strcmp(U->name, V->name) == 0;
}

int tag_cmp(const tag_t* U, const tag_t* V) {
    int c = strcmp(U->name, V->name);

    return (c > 0) - (c < 0);
}

uint64_t tag_hash(const tag_t* U) {
    return string_hash(U ? U->name : NULL);
}

int tag_cmp_void(const void* U, const void* V) {
    return tag_cmp((const tag_t*)U, (const tag_t*)V);
}

uint64_t tag_hash_void(const void* U) {
    return tag_hash((const tag_t*)U);
}

size_t ts_insert_index(tag_set_t* set, tag_t tag) {
    size_t l = 0;
    size_t r = set->size;

    while (l < r) {
        size_t mid = l + (r - l) / 2;
        int _cmp = tag_cmp(&tag, (tag_t*)(set->data)[mid]);
        if (_cmp == 0) return mid;
        if (_cmp < 0) r = mid;
        else l = mid + 1;
    }
    return l;
}

void ts_insert(tag_set_t* set, tag_t tag, size_t insert_ind) {
    void** e_ref = malloc(sizeof(void*));
    if (!e_ref) return;
    *e_ref = (void*)&tag;

    if (insert_ind < set->size) {
        memmove(&set->data[insert_ind + 1],
                &set->data[insert_ind],
                (set->size - insert_ind) * sizeof(tag_t));
    }

    set->data[insert_ind] = e_ref;
    set->size++;
}

bool ts_add(tag_set_t* set, tag_t tag) {
    void** e_ref = malloc(sizeof(void*));
    if (!e_ref) false;
    *e_ref = (void*)&tag;

    size_t insert_index = ts_insert_index(set, tag);
    if (insert_index < set->size && 
        tag_eq(&tag, (tag_t*)set->data[insert_index])) return true;

    if (set->size == set->capacity && 
        !container_resize(set, NULL)) 
        return false;

    ts_insert(set, tag, insert_index);
    return true;
}

bool ts_in(tag_set_t* set, tag_t tag) {
    size_t insert_index = ts_insert_index(set, tag);
    return (insert_index < set->size && 
        tag_eq(&tag, (tag_t*)set->data[insert_index]));
}

tag_set_t* ts_union(tag_set_t* U, tag_set_t* V) {
    if (U->size == 0) {
        tag_set_t* set_union = mk_container(V->size);
        if (!set_union) return NULL;
        memcpy(set_union->data, V->data, V->size * sizeof(tag_t));
        set_union->size = V->size;
        return set_union;
    }
    if (V->size == 0) {
        tag_set_t* set_union= mk_container(U->size);
        if (!set_union) return NULL;
        memcpy(set_union->data, U->data, U->size * sizeof(tag_t));
        set_union->size = U->size;
        return set_union;
    }

    size_t u_ptr = 0;
    size_t v_ptr = 0;
    size_t insert_ind = 0;
    tag_set_t* set_union = mk_container(U->size + V->size);
    if (!set_union) return NULL;

    while (u_ptr < U->size && v_ptr < V->size) {
        tag_t* curr_u_tag = (tag_t*)U->data[u_ptr];
        tag_t* curr_v_tag = (tag_t*)V->data[v_ptr];
        switch (tag_cmp(curr_u_tag, curr_v_tag)) {
            case -1:
                set_union->data[insert_ind++] = (void*)curr_u_tag;
                u_ptr++;
                break;
            case 0: 
                set_union->data[insert_ind++] = (void*)curr_u_tag;
                u_ptr++; v_ptr++;
                break;
            case 1:
                set_union->data[insert_ind++] = (void*)curr_v_tag;
                v_ptr++;
                break;
        }
    }

    while (u_ptr < U->size) set_union->data[insert_ind++] = U->data[u_ptr++];
    while (v_ptr < V->size) set_union->data[insert_ind++] = V->data[v_ptr++];

    set_union->size = insert_ind;
    
    return set_union;
};

tag_set_t* ts_intersection(tag_set_t* U, tag_set_t* V) {
    tag_set_t* set_intersection = mk_container(min(U->size, V->size));
    if (!set_intersection) return NULL;

    tag_set_t* anchor = U->size > V->size ? V : U;
    tag_set_t* ot = U->size > V->size ? U : V;
    size_t insert_ind = 0;
    for (size_t i = 0; i < anchor->size; i++) {
        if (ts_in(ot, *(tag_t*)anchor->data[i])) {
            set_intersection->data[insert_ind++] = anchor->data[i];
        }
    }

    set_intersection->size = insert_ind;
    return set_intersection;
}

int _tag_cmp(const void* u, const void* v) {
    return tag_cmp((tag_t*)u, (tag_t*)v);
}

bool ts_is_valid(tag_set_t* set) {
    return set->size <= set->capacity &&
        is_sorted((void*)set->data, set->size, sizeof(tag_t), &_tag_cmp);
}