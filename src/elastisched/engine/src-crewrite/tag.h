#ifndef TAG_H
#define TAG_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "constants.h"
#include "utils.h"


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

typedef Tag ID; /// an dependency ID is a Tag with an empty description
typedef TagContainer DependencyContainer; 

/**
 * @brief returns True if the names of ```U``` and ```V``` are the same
 */
bool tag_eq(const Tag* U, const Tag* V);

/**
 * @brief comparator for Tag type
 */
int tag_cmp(const Tag* U, const Tag* V);

/**
 * @brief constructs a TagContainer with capacity ```capacity```
 * 
 * @param capacity number of tags that the TagContainer can hold
 * @return TagContainer* 
 */
TagContainer* mk_tag_container(size_t capacity);

/**
 * @brief frees a TagContainer and its data
 *
 * @param container
 */
void tag_container_free(TagContainer* container);

/**
 * @brief resizes a container by doubling capacity and copying memory
 * 
 * @param container
 * @return true if resize successful, false otherwise
 */
bool tag_container_resize(TagContainer* container);

/**
 * @brief Find the insert index
 * 
 * @param set the set to look in.
 * Internally, ```set```->data should be sorted.
 * @param tag the tag to look for
 * @return first index, i, such that tag <= set[j], i<=j<n
 */
size_t ts_insert_index(TagContainer* set, Tag tag);

/**
 * @brief Helper function for inserting tag into a set
 * 
 * @param set
 * @param tag
 * @param insert_ind the index to insert at
 */
void ts_insert(TagContainer* set, Tag tag, size_t insert_ind);

/**
 * @brief Helper function for adding tag into a set
 * 
 * @param set
 * @param tag
 * @return true if no memory failures, false otherwise
 */
bool ts_add(TagContainer* set, Tag tag);

/**
 * @brief Membership checking in O(log|```set```|)
 * 
 * @param set
 * @param tag
 * @return true if ```tag``` is in ```set```
 */
bool ts_in(TagContainer* set, Tag tag);

/**
 * @brief returns a new set containing the set union
 * 
 * @param U
 * @param V 
 * @return TagContainer*
 *
 * @note runtime of this is O(|U| + |V|)
 */
TagContainer* ts_union(TagContainer* U, TagContainer* V);

/**
 * @brief returns a new set containing the set intersection
 * 
 * @param U 
 * @param V 
 * @return TagContainer* 
 *
 * @note runtime of this is O(min(|U|, |V|)*log(max(|U|, |V|)))
 */
TagContainer* ts_intersection(TagContainer* U, TagContainer* V);

/**
 * @brief ensures that the tag set internals meet the appropriate
 * invariants.
 * 
 * @param set
 * @return: true if the invariants are met
 */
bool ts_is_valid(TagContainer* set);

/**
 * @brief pushback tag to tagvec
 * 
 * @param vec 
 * @param tag 
 * @return: true if no memory failures, false otherwise
 */
bool tv_pushback(TagContainer* vec, Tag tag);

#endif
