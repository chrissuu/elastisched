#include "tag.h"

/**
 * @brief returns True if the names of ```U``` and ```V``` are the same
 */
bool tag_eq(Tag* U, Tag* V) {
    return strcmp(U->name, V->name) == 0;
}

int tag_cmp(Tag* U, Tag* V) {
    return strcmp(U->name, V->name);
}

/**
 * @brief sets the description of ```U``` to ```description```
 */
void tag_set_description(Tag* U, char* description) {
    U->description = description;
    return;
}

/**
 * @brief constructs a TagSet with capacity ```capacity```
 * 
 * @param capacity number of tags that the TagSet can hold
 * @return TagSet* 
 */
TagSet* mk_tag_set(size_t capacity) {
    TagSet* tag_set = malloc(sizeof(TagSet));
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

/**
 * @brief resizes a set by doubling capacity and copying memory
 * 
 * @param set set to resize
 * @return true if resize successful, false otherwise
 */
bool resize_set(TagSet* set) {
    set->capacity *= 2;
    Tag* data = malloc(set->capacity * sizeof(Tag));
    if (!data) return false;

    memcpy((void*)data, (void*)set->data, set->size * sizeof(Tag));
    free(set->data);
    set->data = data;
    return true;
}

/**
 * @brief Find the insert index
 * 
 * @param set the set to look in.
 * Internally, ```set```->data should be sorted.
 * @param tag the tag to look for
 * @return size_t 
 */
size_t set_insert_index(TagSet* set, Tag* tag) {
    size_t l = 0;
    size_t r = set->size;

    while (l < r) {
        size_t mid = l + (r - l) / 2;
        int _cmp = tag_cmp(tag, &(set->data)[mid]);
        if (_cmp == 0) return mid;
        if (_cmp < 0) r = mid;
        else l = mid + 1;
    }
    return l;
}


/**
 * @brief Helper function for inserting tag into a set
 * 
 * @param set the set to insert to 
 * @param tag the tag to insert
 * @param insert_ind the index to insert at
 */
void set_insert(TagSet* set, Tag* tag, size_t insert_ind) {
    if (insert_ind < set->size) {
        memmove(&set->data[insert_ind + 1],
                &set->data[insert_ind],
                (set->size - insert_ind) * sizeof(Tag));
    }
    set->data[insert_ind] = *tag;
    set->size++;
}

/**
 * @brief Helper function for addiing tag into a set
 * 
 * @param set set to add to
 * @param tag the tag to add
 * @return true if no memory failures, false otherwise
 */
bool set_add(TagSet* set, Tag* tag) {
    size_t index = set_insert_index(set, tag);
    if (index < set->size && tag_eq(tag, &set->data[index])) return true;

    if (set->size == set->capacity && !resize_set(set)) return false;

    set_insert(set, tag, index);
    return true;
}
