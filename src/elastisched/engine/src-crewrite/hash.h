#ifndef ELASTISCHED_HASH_H
#define ELASTISCHED_HASH_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

static inline uint64_t mix64_hash(uint64_t x) {
    x += 0x9e3779b97f4a7c15ULL;
    x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
    x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
    return x ^ (x >> 31);
}

static inline bool alpha_meets_threshold(size_t num_items, size_t num_buckets) {
    if (num_buckets == 0) return false;
    return (num_items * 4) >= (num_buckets * 3);
}

static inline uint64_t string_hash(const char* str) {
    const uint64_t fnv_offset = 1469598103934665603ULL;
    const uint64_t fnv_prime = 1099511628211ULL;
    uint64_t hash = fnv_offset;
    if (!str) return hash;
    for (const unsigned char* p = (const unsigned char*)str; *p; p++) {
        hash ^= (uint64_t)(*p);
        hash *= fnv_prime;
    }
    return hash;
}

#endif
