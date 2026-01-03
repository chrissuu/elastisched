#include "tag.h"

bool tag_eq(const Tag* U, const Tag* V) {
    return strcmp(U->name, V->name) == 0;
}

int tag_cmp(const Tag* U, const Tag* V) {
    int c = strcmp(U->name, V->name);

    return (c > 0) - (c < 0);
}

TagContainer* mk_tag_container(size_t capacity) {
    TagContainer* tag_set = malloc(sizeof(TagContainer));
    if (!tag_set) return NULL;

    Tag* data = malloc(capacity * sizeof(Tag));
    if (!data) {
        free(tag_set);
        return NULL;
    }

    tag_set->size = 0;
    tag_set->capacity = capacity;
    tag_set->data = data;

    return tag_set;
}

void tag_container_free(TagContainer* set) {
    free((void*)set->data);
    free(set);
    return;
}

bool ts_resize(TagContainer* set) {
    size_t new_capacity = (set->capacity == 0) ? 
        INITIAL_TAGSET_CAPACITY : set->capacity * 2;
    Tag* new_data = realloc(set->data, new_capacity * sizeof(Tag));
    if (!new_data) return false;
    set->data = new_data;
    set->capacity = new_capacity;
    return true;
}

size_t ts_insert_index(TagContainer* set, Tag tag) {
    size_t l = 0;
    size_t r = set->size;

    while (l < r) {
        size_t mid = l + (r - l) / 2;
        int _cmp = tag_cmp(&tag, &(set->data)[mid]);
        if (_cmp == 0) return mid;
        if (_cmp < 0) r = mid;
        else l = mid + 1;
    }
    return l;
}

void ts_insert(TagContainer* set, Tag tag, size_t insert_ind) {
    if (insert_ind < set->size) {
        memmove(&set->data[insert_ind + 1],
                &set->data[insert_ind],
                (set->size - insert_ind) * sizeof(Tag));
    }
    set->data[insert_ind] = tag;
    set->size++;
}

bool ts_add(TagContainer* set, Tag tag) {
    size_t insert_index = ts_insert_index(set, tag);
    if (insert_index < set->size && 
        tag_eq(&tag, &set->data[insert_index])) return true;

    if (set->size == set->capacity && !ts_resize(set)) return false;

    ts_insert(set, tag, insert_index);
    return true;
}

bool ts_in(TagContainer* set, Tag tag) {
    size_t insert_index = ts_insert_index(set, tag);
    return (insert_index < set->size && 
        tag_eq(&tag, &set->data[insert_index]));
}

TagContainer* ts_union(TagContainer* U, TagContainer* V) {
    if (U->size == 0) {
        TagContainer* set_union = mk_tag_set(V->size);
        if (!set_union) return NULL;
        memcpy(set_union->data, V->data, V->size * sizeof(Tag));
        set_union->size = V->size;
        return set_union;
    }
    if (V->size == 0) {
        TagContainer* set_union= mk_tag_set(U->size);
        if (!set_union) return NULL;
        memcpy(set_union->data, U->data, U->size * sizeof(Tag));
        set_union->size = U->size;
        return set_union;
    }

    size_t u_ptr = 0;
    size_t v_ptr = 0;
    size_t insert_ind = 0;
    TagContainer* set_union = mk_tag_container(U->size + V->size);
    if (!set_union) return NULL;

    while (u_ptr < U->size && v_ptr < V->size) {
        Tag* curr_u_tag = &U->data[u_ptr];
        Tag* curr_v_tag = &V->data[v_ptr];
        switch (tag_cmp(curr_u_tag, curr_v_tag)) {
            case -1:
                set_union->data[insert_ind++] = *curr_u_tag;
                u_ptr++;
                break;
            case 0: 
                set_union->data[insert_ind++] = *curr_u_tag;
                u_ptr++; v_ptr++;
                break;
            case 1:
                set_union->data[insert_ind++] = *curr_v_tag;
                v_ptr++;
                break;
        }
        set_union->size++;
    }

    while (u_ptr < U->size) set_union->data[insert_ind++] = U->data[u_ptr++];
    while (v_ptr < V->size) set_union->data[insert_ind++] = V->data[v_ptr++];

    set_union->size = insert_ind;
    
    return set_union;
};

TagContainer* ts_intersection(TagContainer* U, TagContainer* V) {
    TagContainer* set_intersection = mk_tag_container(min(U->capacity, V->capacity));

    size_t u_ptr = 0;
    size_t v_ptr = 0;
    size_t insert_ind = 0;

    while(u_ptr < U->size) {
        Tag* curr_u_tag = &U->data[u_ptr];
        Tag* curr_v_tag = &V->data[v_ptr];
        if (tag_eq(curr_u_tag, curr_v_tag)) set_intersection->data[insert_ind++] = U->data[u_ptr++];
    }
}
