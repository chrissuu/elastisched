#include "map.h"

#include "constants.h"
#include "dll.h"

typedef struct bucket_vec {
    size_t size;
    size_t capacity;
    dll* bucket;
} bucket_vec;

struct map {
    size_t num_keys;
    size_t num_values;
    size_t capacity;
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

    dict->num_keys = 0;
    dict->num_values = 0;
    dict->capacity = INITIAL_MAP_CAPACITY;
    dict->hash_fn = hash_fn;
    dict->cmp_fn = cmp_fn;
    dict->free_fn = free_fn;
    
}