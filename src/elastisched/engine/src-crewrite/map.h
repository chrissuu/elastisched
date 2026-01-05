#ifndef MAP_H
#define MAP_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "constants.h"
#include "dll.h"
#include "vec.h"

typedef struct map map;
typedef struct map set;

typedef struct item {
    void* key;
    void* value;
} item;

typedef struct vec_items {
    size_t size;
    size_t capacity;
    item** data;
} vec_items;

map* mk_map(uint64_t (*hash_fn)(void* e), 
    int (*cmp_fn)(void* u, void* v),
    void (*free_fn)(void* e));

void map_free(map* dict);

void map_insert(map* dict, void* key, void* value);
bool map_in(map* dict, void* key);
void* map_get(map* dict, void* key);
void map_delete(map* dict, void* key);
size_t map_size(map* dict);
vec_items* map_items(map* dict);

#endif