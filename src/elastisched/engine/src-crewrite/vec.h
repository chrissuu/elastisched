#ifndef ELASTISCHED_VEC_H
#define ELASTISCHED_VEC_H

#include <stdlib.h>
#include "constants.h"

/**
 * vec.h
 *
 * Macros for working with arbitrary C vectors
 */

#define vec_realloc(vec) \
        do { \
            vec->data = realloc(vec->data, vec->capacity * sizeof(*vec->data)); \
        } while (0)

#define vec_pushback(vec, e) \
        do { \
            if (vec->size >= vec->capacity) { \
                if (vec->size == 0) vec->capacity = INITIAL_CONTAINER_CAPACITY; \
                else vec->capacity *= 2; \
                vec->data = realloc(vec->data, vec->capacity * sizeof(*vec->data)); \
            } \
            vec->data[vec->size++] = e; \
        } while (0)

#define vec_clear(vec) \
        do { \
            vec->size = 0; \
        } while (0)

#endif
