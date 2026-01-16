#ifndef ELASTISCHED_MAP_H
#define ELASTISCHED_MAP_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "dll.h"
#include "container.h"

typedef struct map map;
typedef struct map set;

typedef struct item {
    void* key;
    void* value;
} item;

#define INITIAL_MAP_CAPACITY 32

typedef container_t vec_items_t;

map* mk_map(uint64_t (*hash_fn)(const void* e),
    int (*cmp_fn)(const void* u, const void* v),
    void (*free_fn)(void* e));

void map_free(map* dict);

void map_insert(map* dict, void* key, void* value);
bool map_in(map* dict, void* key);
void* map_get(map* dict, void* key);
void map_delete(map* dict, void* key);
size_t map_size(map* dict);
vec_items_t* map_items(map* dict);

#endif
