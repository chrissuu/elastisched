#ifndef TAG_H
#define TAG_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "constants.h"

/**
 * Tag.h
 * 
 * Definitions for working with the Tag and TagContainer structs.
 * 
 * TagContainer is a simple container struct which allows for
 * functions to operate on it and implement Sets and Vectors.
 * 
 * TagSet in practice does not get very large nor updated
 * often so we opt for a simple vector-based sorted set.
 * 
 * Two tags are equivalent if they share the same name,
 * not necessarily the same description.
 * 
 * This allows for O(logn) search time and membership
 * checking, O(n) set difference and O(n) time to add
 * to the set.
 */


typedef struct Tag {
    char* name;
    char* description;
} Tag;

typedef struct TagContainer {
    Tag* data;
    size_t size;
    size_t capacity;
} TagContainer;

/**
 * @brief returns True if the names of ```U``` and ```V``` are the same
 */
bool tag_eq(Tag* U, Tag* V);

/**
 * @brief comparator for Tag type
 */
int tag_cmp(Tag* U, Tag* V);

/**
 * @brief constructs a TagContainer with capacity ```capacity```
 * 
 * @param capacity number of tags that the TagContainer can hold
 * @return TagContainer* 
 */
TagContainer* mk_tag_container(size_t capacity);

/**
 * @brief resizes a set by doubling capacity and copying memory
 * 
 * @param set set to resize
 * @return true if resize successful, false otherwise
 */
bool ts_resize(TagContainer* set);

/**
 * @brief Find the insert index
 * 
 * @param set the set to look in.
 * Internally, ```set```->data should be sorted.
 * @param tag the tag to look for
 * @return first index, i, such that tag <= set[j], i<=j<n
 */
size_t ts_insert_index(TagContainer* set, Tag* tag);

/**
 * @brief Helper function for inserting tag into a set
 * 
 * @param set the set to insert to 
 * @param tag the tag to insert
 * @param insert_ind the index to insert at
 */
void ts_insert(TagContainer* set, Tag* tag, size_t insert_ind);

/**
 * @brief Helper function for adding tag into a set
 * 
 * @param set set to add to
 * @param tag the tag to add
 * @return true if no memory failures, false otherwise
 */
bool ts_add(TagContainer* set, Tag* tag);

/**
 * @brief Membership checking in O(logn)
 * 
 * @param set set to check membership of
 * @param tag tag to check membership in
 * @return true if ```tag``` is in ```set```
 */
bool ts_in(TagContainer* set, Tag* tag);

TagContainer* ts_union(TagContainer* U, TagContainer* V);
TagContainer* ts_intersection(TagContainer* U, TagContainer* V);

bool tv_resize(TagContainer* vec);
bool tv_pushback(TagContainer* vec, Tag* tag);


#endif
