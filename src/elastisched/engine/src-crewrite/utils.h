#include <stdbool.h>
#include <stdlib.h>
#include "constants.h"

size_t min(size_t u, size_t v);
size_t max(size_t u, size_t v);
bool is_sorted(const void* base, size_t count, size_t elem_size,
               int (*cmp)(const void*, const void*));