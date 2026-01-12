#include "utils.h"

size_t min(size_t u, size_t v) {
    return u > v ? v : u;
}

size_t max(size_t u, size_t v) {
    return u > v ? u: v;
}

bool is_sorted(const void* base, size_t count, size_t elem_size,
               int (*cmp)(const void*, const void*)) {
    if (count < 2) return true;

    const char* bytes = (const char*)base;
    for (size_t i = 1; i < count; i++) {
        const void* prev = bytes + (i - 1) * elem_size;
        const void* curr = bytes + i * elem_size;
        if (cmp(prev, curr) > 0) return false;
    }
    return true;
}
