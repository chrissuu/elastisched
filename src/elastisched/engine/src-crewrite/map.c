#include "map.h"

/**
 * @brief Struct which manages map buckets
 * in chaining hash-map. 
 * 
 * Note: that size may not always equal 
 * num_items since there may be collisions.
 */
typedef struct bucket_vec {
    size_t size; /// num of occupied data
    size_t capacity; /// length of data
    size_t num_items;
    dll** data;
} bucket_vec;

struct map {
    uint64_t (*hash_fn)(void* e);
    int (*cmp_fn)(void* u, void* v);
    void (*free_fn)(void* e);
    bucket_vec* buckets;
};

map* mk_map(uint64_t (*hash_fn)(void* e), 
    int (*cmp_fn)(void* u, void* v), 
    void (*free_fn)(void* e)) 
{
    map* dict = malloc(sizeof(map));
    if (!dict) return NULL;

    dict->buckets = malloc(INITIAL_MAP_CAPACITY * sizeof(bucket_vec));
    if (!dict->buckets) {
        free(dict);
        return NULL;
    }

    dict->buckets->size = 0;
    dict->buckets->num_items = 0;
    dict->buckets->capacity = INITIAL_MAP_CAPACITY;
    dict->buckets->data = NULL;

    dict->hash_fn = hash_fn;
    dict->cmp_fn = cmp_fn;
    dict->free_fn = free_fn;

    return dict;
}

void map_free(map* dict) {
    return;
}

void _map_insert(map* dict, void* key, void* value) {
    size_t h = (size_t)dict->hash_fn(key);
    size_t insert_index = mix64_hash(h) & (dict->buckets->capacity - 1);
    dll* deque = dict->buckets->data[insert_index];

    if (deque) {
        item* kv = malloc(sizeof(item));
        if (!kv) return;

        kv->key = key; kv->value = value;
        dll_append(deque, (void*)kv);
    } else {
        item* kv = malloc(sizeof(item));
        if (!kv) return;

        kv->key = key; kv->value = value;
        dll* bucket = mk_dll();
        dll_append(bucket, (void*)kv);
        *(&deque) = bucket;
        dict->buckets->size++;
    }

    dict->buckets->num_items++;
    return;
}

void rebuild_map(map* dict) {
    map* temp_dict = malloc(sizeof(map));
    if (!temp_dict) {
        fprintf(stderr, "error: malloc failed during map rebuild\n");
        return;
    }

    temp_dict->hash_fn = dict->hash_fn;
    temp_dict->cmp_fn = dict->cmp_fn;
    temp_dict->free_fn = dict->free_fn;
    
    for (size_t i = 0; i < dict->buckets->capacity; i++) {
        dll* deque = dict->buckets->data[i];
        if (deque) {
            dll_node* curr = dll_head(deque);
            while (curr) {
                item* curr_item = (item*)dll_node_get_value(curr);
                _map_insert(temp_dict, (void*)curr_item->key, 
                    (void*)curr_item->value);
                curr = dll_next(curr);
            }

            dll_free(deque, NULL);
        }
    }

    dict = temp_dict;
    return;
}

void map_insert(map* dict, void* key, void* value) {
    if (map_in(dict, key)) return;

    if (alpha_meets_threshold(dict->buckets->num_items, 
        dict->buckets->size)) {
        dict->buckets->capacity *= 2;
        rebuild_map(dict);
    }
    _map_insert(dict, key, value);
    return;
}

bool map_in(map* dict, void* key) {
    size_t h = (size_t)dict->hash_fn(key);
    size_t index = mix64_hash(h) & (dict->buckets->capacity - 1);
    dll* deque = dict->buckets->data[index];

    if (deque) {
        dll_node* curr = dll_head(deque);
        while (curr) {
            item* curr_item = (item*)dll_node_get_value(curr);
            if (dict->cmp_fn(key, curr_item->key) == 0) {
                return true;
            }
            curr = dll_next(curr);
        }
    }
    return false;
}

void* map_get(map* dict, void* key) {
    size_t h = (size_t)dict->hash_fn(key);
    size_t index = mix64_hash(h) & (dict->buckets->capacity - 1);
    dll* deque = dict->buckets->data[index];

    if (deque) {
        dll_node* curr = dll_head(deque);
        while (curr) {
            item* curr_item = (item*)dll_node_get_value(curr);
            if (dict->cmp_fn(key, curr_item->key) == 0) {
                return curr_item->value;
            }
            curr = dll_next(curr);
        }
    }
    return NULL;
}

void map_delete(map* dict, void* key) {
    size_t h = (size_t)dict->hash_fn(key);
    size_t index = mix64_hash(h) & (dict->buckets->capacity - 1);
    dll* deque = dict->buckets->data[index];

    if (deque) {
        dll_node* curr = dll_head(deque);
        while (curr) {
            item* curr_item = (item*)dll_node_get_value(curr);
            if (dict->cmp_fn(key, curr_item->key) == 0) {
                dll_remove(deque, curr);
                return;
                // TODO: need to free something here
            }
            curr = dll_next(curr);
        }
    }
    return;
}

size_t map_size(map* dict) {
    return dict->buckets->num_items;
}

vec_items* map_items(map* dict) {
    vec_items* items = malloc(sizeof(vec_items));
    if (!items) return NULL;

    for (size_t i = 0; i < dict->buckets->capacity; i++) {
        dll* deque = dict->buckets->data[i];
        if (deque) {
            dll_node* curr = dll_head(deque);
            while (curr) {
                item* curr_item = (item*)dll_node_get_value(curr);
                vec_pushback(items, curr_item);
                curr = dll_next(curr);
            }
        }
    }

    return items;
}