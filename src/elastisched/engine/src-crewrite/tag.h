#ifndef TAG_H
#define TAG_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Tag.h
 * 
 * Definitions for working with the Tag and TagSet structs.
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

typedef struct TagSet {
    Tag* data;
    size_t size;
    size_t capacity;
} TagSet;

/**
 * @brief returns True if the names of ```U``` and ```V``` are the same
 */
bool tag_eq(Tag* U, Tag* V);

/**
 * @brief comparator for Tag type
 */
int tag_cmp(Tag* U, Tag* V);

/**
 * @brief sets the description of ```U``` to ```description```
 */
void tag_set_description(Tag* U, char* description);

/**
 * @brief constructs a TagSet with capacity ```capacity```
 * 
 * @param capacity number of tags that the TagSet can hold
 * @return TagSet* 
 */
TagSet* mk_tag_set(size_t capacity);

/**
 * @brief resizes a set by doubling capacity and copying memory
 * 
 * @param set set to resize
 * @return true if resize successful, false otherwise
 */
bool resize_set(TagSet* set);

/**
 * @brief Find the insert index
 * 
 * @param set the set to look in.
 * Internally, ```set```->data should be sorted.
 * @param tag the tag to look for
 * @return size_t 
 */
size_t set_insert_index(TagSet* set, Tag* tag);

/**
 * @brief Helper function for inserting tag into a set
 * 
 * @param set the set to insert to 
 * @param tag the tag to insert
 * @param insert_ind the index to insert at
 */
void set_insert(TagSet* set, Tag* tag, size_t insert_ind);

/**
 * @brief Helper function for addiing tag into a set
 * 
 * @param set set to add to
 * @param tag the tag to add
 * @return true if no memory failures, false otherwise
 */
bool set_add(TagSet* set, Tag* tag);

#endif
