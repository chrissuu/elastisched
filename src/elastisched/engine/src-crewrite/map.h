#ifndef MAP_H
#define MAP_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "constants.h"

typedef struct map map;

map* mk_map(uint64_t (*hash_fn)(void* e), 
    int (*cmp_fn)(void* u, void* v),
    void (*free_fn)(void* e));

void map_free(map* dict);

void map_insert(map* dict, void* key, void* value);
bool map_in(map* dict, void* key);
bool map_get(map* dict, void* key);
bool map_size(map* dict);

#endif