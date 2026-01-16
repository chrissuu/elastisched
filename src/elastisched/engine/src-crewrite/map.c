#include "map.h"
#include "hash.h"
#include <stdio.h>

/**
 * @brief Struct which manages map buckets
 * in chaining hash-map.
 *
 * Note: size is number of occupied buckets; num_items is total entries.
 */
typedef struct bucket_vec {
    size_t num_items;
    container_t* data;
} bucket_vec;

struct map {
    uint64_t (*hash_fn)(const void* e);
    int (*cmp_fn)(const void* u, const void* v);
    void (*free_fn)(void* e);
    bucket_vec* buckets;
};

static bucket_vec* mk_buckets(size_t capacity) {
    bucket_vec* buckets = malloc(sizeof(bucket_vec));
    if (!buckets) return NULL;

    container_t* data = malloc(sizeof(container_t));
    if (!data) {
        free(buckets);
        return NULL;
    }

    dll** bucket_data = calloc(capacity, sizeof(dll*));
    if (!bucket_data) {
        free(data);
        free(buckets);
        return NULL;
    }

    data->size = 0;
    data->capacity = capacity;
    data->data = (void**)bucket_data;

    buckets->num_items = 0;
    buckets->data = data;
    return buckets;
}

static void free_item_value(void* value, void (*free_fn)(void*)) {
    item* kv = (item*)value;
    if (free_fn) free_fn(kv->value);
    free(kv);
}

static size_t bucket_index(map* dict, void* key) {
    size_t h = (size_t)dict->hash_fn(key);
    return mix64_hash(h) & (dict->buckets->data->capacity - 1);
}

static bool map_insert_bucket(map* dict, void* key, void* value) {
    size_t index = bucket_index(dict, key);
    dll* deque = (dll*)dict->buckets->data->data[index];

    item* kv = malloc(sizeof(item));
    if (!kv) return false;
    kv->key = key;
    kv->value = value;

    if (!deque) {
        deque = mk_dll();
        if (!deque) {
            free(kv);
            return false;
        }
        dict->buckets->data->data[index] = deque;
        dict->buckets->data->size++;
    }

    dll_append(deque, (void*)kv);
    dict->buckets->num_items++;
    return true;
}

static dll_node* map_find_node(map* dict, void* key) {
    size_t index = bucket_index(dict, key);
    dll* deque = (dll*)dict->buckets->data->data[index];
    if (!deque) return NULL;

    dll_node* curr = dll_head(deque);
    while (curr) {
        item* curr_item = (item*)dll_node_get_value(curr);
        if (dict->cmp_fn(key, curr_item->key) == 0) return curr;
        curr = dll_next(curr);
    }
    return NULL;
}

static void map_rebuild(map* dict, size_t new_capacity) {
    bucket_vec* new_buckets = mk_buckets(new_capacity);
    if (!new_buckets) {
        fprintf(stderr, "error: malloc failed during map rebuild\n");
        return;
    }

    bucket_vec* old_buckets = dict->buckets;
    dict->buckets = new_buckets;

    for (size_t i = 0; i < old_buckets->data->capacity; i++) {
        dll* deque = (dll*)old_buckets->data->data[i];
        if (!deque) continue;

        while (dll_size(deque)) {
            item* kv = (item*)dll_popleft(deque);
            map_insert_bucket(dict, kv->key, kv->value);
            free(kv);
        }

        free(deque);
    }

    free(old_buckets->data->data);
    free(old_buckets->data);
    free(old_buckets);
}

map* mk_map(uint64_t (*hash_fn)(const void* e),
    int (*cmp_fn)(const void* u, const void* v),
    void (*free_fn)(void* e))
{
    if (!hash_fn || !cmp_fn) return NULL;
    map* dict = malloc(sizeof(map));
    if (!dict) return NULL;

    dict->buckets = mk_buckets(INITIAL_MAP_CAPACITY);
    if (!dict->buckets) {
        free(dict);
        return NULL;
    }

    dict->hash_fn = hash_fn;
    dict->cmp_fn = cmp_fn;
    dict->free_fn = free_fn;

    return dict;
}

void map_free(map* dict) {
    if (!dict) return;

    for (size_t i = 0; i < dict->buckets->data->capacity; i++) {
        dll* deque = (dll*)dict->buckets->data->data[i];
        if (!deque) continue;

        while (dll_size(deque)) {
            item* kv = (item*)dll_popleft(deque);
            free_item_value(kv, dict->free_fn);
        }

        free(deque);
    }

    free(dict->buckets->data->data);
    free(dict->buckets->data);
    free(dict->buckets);
    free(dict);
}

void map_insert(map* dict, void* key, void* value) {
    if (!dict) return;

    dll_node* existing = map_find_node(dict, key);
    if (existing) {
        item* kv = (item*)dll_node_get_value(existing);
        if (dict->free_fn) dict->free_fn(kv->value);
        kv->value = value;
        return;
    }

    if (alpha_meets_threshold(dict->buckets->num_items, dict->buckets->data->capacity)) {
        map_rebuild(dict, dict->buckets->data->capacity * 2);
    }

    map_insert_bucket(dict, key, value);
}

bool map_in(map* dict, void* key) {
    return map_find_node(dict, key) != NULL;
}

void* map_get(map* dict, void* key) {
    dll_node* node = map_find_node(dict, key);
    if (!node) return NULL;
    item* kv = (item*)dll_node_get_value(node);
    return kv->value;
}

void map_delete(map* dict, void* key) {
    if (!dict) return;
    size_t index = bucket_index(dict, key);
    dll* deque = (dll*)dict->buckets->data->data[index];
    if (!deque) return;

    dll_node* curr = dll_head(deque);
    while (curr) {
        item* curr_item = (item*)dll_node_get_value(curr);
        if (dict->cmp_fn(key, curr_item->key) == 0) {
            free_item_value(curr_item, dict->free_fn);
            dll_remove(deque, curr);
            dict->buckets->num_items--;
            if (dll_size(deque) == 0) {
                dict->buckets->data->data[index] = NULL;
                dict->buckets->data->size--;
                free(deque);
            }
            return;
        }
        curr = dll_next(curr);
    }
}

size_t map_size(map* dict) {
    return dict ? dict->buckets->num_items : 0;
}

vec_items_t* map_items(map* dict) {
    if (!dict) return NULL;
    vec_items_t* items = malloc(sizeof(vec_items_t));
    if (!items) return NULL;

    items->size = 0;
    items->capacity = 0;
    items->data = NULL;

    for (size_t i = 0; i < dict->buckets->data->capacity; i++) {
        dll* deque = (dll*)dict->buckets->data->data[i];
        if (!deque) continue;

        dll_node* curr = dll_head(deque);
        while (curr) {
            item* curr_item = (item*)dll_node_get_value(curr);
            if (!container_append(items, curr_item)) {
                container_free(items, NULL);
                return NULL;
            }
            curr = dll_next(curr);
        }
    }

    return items;
}
