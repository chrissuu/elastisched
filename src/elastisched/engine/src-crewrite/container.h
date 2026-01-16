#ifndef CONTAINER_H
#define CONTAINER_H

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

/**
 * @brief Abstract data container which also adds (optional) safe dynamic array semantics
 * 
 * container_t is the primary struct of interest here. container.h provides a
 * mostly safe (and simple) API for interacting with the container_t struct.
 */

#define INITIAL_CONTAINER_CAPACITY 16

typedef struct container {
    size_t size;
    size_t capacity;
    void** data;
} container_t;

container_t* mk_container(size_t capacity);

/**
 * @brief frees a container, optionally the elements if free_fn is not NULL
 * 
 * @param container 
 */
void container_free(container_t* container, void (*free_fn)(void*));

/**
 * @brief Resize the container
 * 
 * @param container 
 * @param capacity optional value which the function will use if it is not NULL.
 * @return true if resize was successful
 * @return false otherwise
 */
bool container_resize(container_t* container, size_t* capacity);

/**
 * @brief Appends to a container. Runs in O(1) amortized work.
 * 
 * @param container 
 * @param e 
 * @return true if container[size-1] = e
 * @return false otherwise (this can include memory failures, bad inputs, etc)
 */
bool container_append(container_t* container, void* e);

/**
 * @brief inserts to a container. Runs in O(n) work.
 * 
 * @param container 
 * @param e 
 * @param insert_ind 
 * @return true if container[insert_index] = e
 * @return false otherwise
 */
bool container_insert(container_t* container, void* e, size_t insert_ind);

#endif
