#include "topo_sort.h"

#define id_hash tag_hash
#define id_cmp tag_cmp

DependencyCheckResult* check_dependency_violations(const Schedule* schedule) {
    DependencyCheckResult* result = malloc(sizeof(DependencyCheckResult));
    if (!result) return NULL;
    result->has_violations = false;
    result->violations = NULL;
    result->has_cyclic_dependencies = false;

    JobVec* scheduled_jobs = schedule->scheduled_jobs;

    if (scheduled_jobs->size == 0) return result;

    map* id_to_job = mk_map(&id_hash, &id_cmp, NULL);
    for (size_t i = 0; i < scheduled_jobs->size; i++) {
        map_insert(id_to_job, 
            (void*)&scheduled_jobs->data[i].id, 
            (void*)&scheduled_jobs->data[i]);
    }

    map* adj_list = mk_map(&id_hash, &id_cmp, NULL);
    map* in_degree = mk_map(&id_hash, &id_cmp, NULL);

    for (size_t i = 0; i < scheduled_jobs->size; i++) {
        ID* job_id = &scheduled_jobs->data[i].id;
        JobVec* job_vec = malloc(sizeof(JobVec));
        size_t* in_degree = malloc(sizeof(size_t));
        map_insert(adj_list, (void*)job_id, (void*)job_vec);
        map_insert(in_degree, (void*)job_id, (void*)in_degree);
    }

    for (size_t i = 0; i < scheduled_jobs->size; i++) {
        ID* job_this = &scheduled_jobs->data[i].id;
        DependencyContainer* job_dependencies = scheduled_jobs->data[i].dependency_set;
        for (size_t j = 0; j < job_dependencies->size; j++) {
            ID* job_ot = &job_dependencies->data[j];
            if (map_in(id_to_job, (void*)job_ot)) {
                map_insert(adj_list, (void*)job_ot, (void*)job_this);
                map_insert(in_degree, (void*)job_this, 
                    *(size_t*)map_get(in_degree, (void*)job_this) + 1);
            }
        }
    }

    dll* queue = mk_dll();
    DependencyContainer* topo_order = mk_tag_container(8);
    vec_items* items = map_items(in_degree);

    for (size_t i = 0; i < items->size; i++) {
        if (*(size_t*)items->data[i]->value == 0) {
            dll_append(queue, items->data[i]->key);
        }
    }

    while (dll_size(queue)) {
        ID* curr_id = (ID*)dll_node_get_value(dll_popleft(queue));
        vec_pushback(topo_order, *curr_id);

        for (size_t i = 0; i < ((JobVec*)map_get(adj_list, (void*)curr_id))->size; i++) {
            
        }
    }
}