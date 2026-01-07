#include "topo_sort.h"
#include "hash.h"
#include <stdlib.h>

#define id_hash tag_hash_void
#define id_cmp tag_cmp_void

static DependencyViolationContainer* mk_violation_container(size_t capacity) {
    DependencyViolationContainer* container = malloc(sizeof(DependencyViolationContainer));
    if (!container) return NULL;
    container->dependency_violations = NULL;
    container->size = 0;
    container->capacity = 0;

    if (capacity == 0) return container;
    container->dependency_violations = calloc(capacity, sizeof(DependencyViolation));
    if (!container->dependency_violations) {
        free(container);
        return NULL;
    }
    container->capacity = capacity;
    return container;
}

static bool violation_container_push(DependencyViolationContainer* container,
    DependencyViolation violation) {
    if (container->size == container->capacity) {
        size_t new_capacity = container->capacity == 0 ? 4 : container->capacity * 2;
        DependencyViolation* new_data = realloc(container->dependency_violations,
            new_capacity * sizeof(DependencyViolation));
        if (!new_data) return false;
        container->dependency_violations = new_data;
        container->capacity = new_capacity;
    }

    container->dependency_violations[container->size++] = violation;
    return true;
}

static void free_tag_container(void* value) {
    TagContainer* container = (TagContainer*)value;
    if (!container) return;
    free(container->data);
    free(container);
}

static void free_violation_container(DependencyViolationContainer* container) {
    if (!container) return;
    for (size_t i = 0; i < container->size; i++) {
        tag_container_free(container->dependency_violations[i].violated_dependencies);
    }
    free(container->dependency_violations);
    free(container);
}

DependencyCheckResult* check_dependency_violations(const Schedule* schedule) {
    DependencyCheckResult* result = malloc(sizeof(DependencyCheckResult));
    if (!result) return NULL;
    result->has_violations = false;
    result->violations = NULL;
    result->has_cyclic_dependencies = false;

    if (!schedule || !schedule->scheduled_jobs || schedule->scheduled_jobs->size == 0) {
        return result;
    }

    JobVec* scheduled_jobs = schedule->scheduled_jobs;

    map* index_map = mk_map(&id_hash, &id_cmp, free);
    if (!index_map) return result;

    for (size_t i = 0; i < scheduled_jobs->size; i++) {
        size_t* index = malloc(sizeof(size_t));
        if (!index) continue;
        *index = i;
        map_insert(index_map, (void*)&scheduled_jobs->data[i].id, (void*)index);
    }

    DependencyViolationContainer* violations = mk_violation_container(4);
    if (!violations) {
        map_free(index_map);
        return result;
    }

    for (size_t i = 0; i < scheduled_jobs->size; i++) {
        Job* job = &scheduled_jobs->data[i];
        DependencyContainer* deps = job->dependency_set;
        if (!deps || deps->size == 0) continue;

        TagContainer* violated = mk_tag_container(0);
        if (!violated) continue;

        for (size_t j = 0; j < deps->size; j++) {
            ID* dep_id = &deps->data[j];
            size_t* dep_index = (size_t*)map_get(index_map, (void*)dep_id);
            if (!dep_index) continue;
            if (*dep_index > i) {
                tv_pushback(violated, *dep_id);
            }
        }

        if (violated->size > 0) {
            DependencyViolation violation = {
                .job_id = job->id,
                .violated_dependencies = violated
            };
            if (!violation_container_push(violations, violation)) {
                tag_container_free(violated);
            }
        } else {
            tag_container_free(violated);
        }
    }

    if (violations->size > 0) {
        result->has_violations = true;
        result->violations = violations;
    } else {
        free_violation_container(violations);
    }

    map_free(index_map);

    map* adj_list = mk_map(&id_hash, &id_cmp, free_tag_container);
    map* in_degree = mk_map(&id_hash, &id_cmp, free);
    if (!adj_list || !in_degree) {
        map_free(adj_list);
        map_free(in_degree);
        return result;
    }

    for (size_t i = 0; i < scheduled_jobs->size; i++) {
        ID* job_id = &scheduled_jobs->data[i].id;
        TagContainer* neighbors = mk_tag_container(0);
        size_t* degree = calloc(1, sizeof(size_t));
        if (!neighbors || !degree) {
            free_tag_container(neighbors);
            free(degree);
            continue;
        }
        map_insert(adj_list, (void*)job_id, (void*)neighbors);
        map_insert(in_degree, (void*)job_id, (void*)degree);
    }

    for (size_t i = 0; i < scheduled_jobs->size; i++) {
        Job* job = &scheduled_jobs->data[i];
        ID* job_id = &job->id;
        DependencyContainer* deps = job->dependency_set;
        if (!deps) continue;

        for (size_t j = 0; j < deps->size; j++) {
            ID* dep_id = &deps->data[j];
            if (!map_in(adj_list, (void*)dep_id)) continue;
            TagContainer* neighbors = (TagContainer*)map_get(adj_list, (void*)dep_id);
            if (!neighbors) continue;
            tv_pushback(neighbors, *job_id);

            size_t* degree = (size_t*)map_get(in_degree, (void*)job_id);
            if (degree) (*degree)++;
        }
    }

    dll* queue = mk_dll();
    TagContainer* topo_order = mk_tag_container(scheduled_jobs->size);
    if (!queue || !topo_order) {
        dll_free(queue, NULL);
        tag_container_free(topo_order);
        map_free(adj_list);
        map_free(in_degree);
        return result;
    }

    vec_items* items = map_items(in_degree);
    if (items) {
        for (size_t i = 0; i < items->size; i++) {
            if (*(size_t*)items->data[i]->value == 0) {
                dll_append(queue, items->data[i]->key);
            }
        }
    }

    while (dll_size(queue)) {
        ID* curr_id = (ID*)dll_popleft(queue);
        if (!curr_id) continue;
        tv_pushback(topo_order, *curr_id);

        TagContainer* neighbors = (TagContainer*)map_get(adj_list, (void*)curr_id);
        if (!neighbors) continue;

        for (size_t i = 0; i < neighbors->size; i++) {
            ID* neighbor_id = &neighbors->data[i];
            size_t* degree = (size_t*)map_get(in_degree, (void*)neighbor_id);
            if (!degree || *degree == 0) continue;
            (*degree)--;
            if (*degree == 0) {
                dll_append(queue, neighbor_id);
            }
        }
    }

    if (topo_order->size < scheduled_jobs->size) {
        result->has_cyclic_dependencies = true;
    }

    if (items) {
        free(items->data);
        free(items);
    }
    dll_free(queue, NULL);
    tag_container_free(topo_order);
    map_free(adj_list);
    map_free(in_degree);

    return result;
}
