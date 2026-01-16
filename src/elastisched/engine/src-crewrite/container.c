#include "container.h"

container_t* mk_container(size_t capacity) {
    container_t* container = malloc(sizeof(container_t));
    if (!container) return NULL;

    void** data = NULL;
    if (capacity != 0) {
        data = malloc(capacity * sizeof(void*));
        if (!data) {
            free(container);
            return NULL;
        }
    }

    container->size = 0;
    container->capacity = (!container->capacity) ? INITIAL_CONTAINER_CAPACITY : capacity;
    container->data = data;

    return container;
}

void container_free(container_t* container, void (*free_fn)(void*)) {
    if (!container) return;
    if (free_fn) {
        for (size_t i = 0; i < container->size; ++i) {
            free_fn(container->data[i]);
        }
    }

    free((void*)container->data);
    free(container);
}

bool container_resize(container_t* container, size_t* capacity) {
    if (!container) return false;
    if (capacity && *capacity < container->size) return false;

    /* if capacity is not NULL, use capacity. Otherwise, check 
     * if container->capacity is 0. If true, use 
     * ```INITIAL_CONTAINER_CAPACITY```, otherwise, double 
     * container capacity. */
    size_t new_capacity = capacity ? 
        *capacity : 
        ((container->capacity == 0) ? 
            INITIAL_CONTAINER_CAPACITY : container->capacity * 2);

    void** new_data = realloc(container->data, new_capacity * sizeof(void*));
    if (!new_data) return false;

    container->data = new_data;
    container->capacity = new_capacity;
    return true;
}

bool container_append(container_t* container, void* e) {
    if (!container) return false;
    if (container->size == container->capacity) {
        if (!container_resize(container, NULL)) return false;
    }
    container->data[container->size++] = e;
    return true;
}

bool container_insert(container_t* container, void* e, size_t insert_ind) {
    if (!container) return false;
    if (insert_ind > container->size) return false;

    if (container->size == container->capacity) {
        if (!container_resize(container, NULL)) return false;
    }

    if (insert_ind < container->size) {
        memmove(&container->data[insert_ind + 1],
                &container->data[insert_ind],
                (container->size - insert_ind) * sizeof(void*));
    }

    container->data[insert_ind] = e;
    container->size++;
    return true;
}