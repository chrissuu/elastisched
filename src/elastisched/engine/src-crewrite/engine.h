#ifndef ENGINE_H
#define ENGINE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vec.h"
#include "set.h"
#include "map.h"

#include "constants.h"
#include "utils.h"

/**
 * ============================================================================
 * ===================  ELASTISCHED CORE SCHEDULING ENGINE  ===================
 * ============================================================================
 *
 * @note this used to be written in C++, but it was re-written in C since what I
 * got out of C++ was too much machinery for something that ended up being quite
 * simple to implement.
 *
 * The scheduling engine uses simulated annealing (see:
 * https://en.wikipedia.org/wiki/Simulated_annealing) to find schedules which
 * minimize some cost function.
 *
 * Cost functions are intentionally non-opinionated, since preference learning
 * (see ```learning```) should eventually learn user preferences which cannot be
 * directly implemented by code.
 *
 * We call these implemented by the scheduling engine as "primitive costs
 * functions", and exist to ensure validity, safety, and general optimality of
 * the scheduled blobs / recurrences.
 *
 * Currently, we implement the following costs:
 *
 *      - illegal schedule cost
 *      - overlap cost
 *      - split cost
 *
 * ============================================================================
 * Illegal Schedule Cost
 * ============================================================================
 *
 * Illegal schedule cost is returned whenever a schedule is deemed illegal by
 * the scheduler. A schedule is considered illegal if any of the following are
 * true:
 *
 *      - a blob's scheduled tr overlaps a blob's schedulable tr
 *      - a non overlappable blob overlaps with another blob
 *      - a blob's dependencies are not met
 *
 * @note currently, the frontend / backend of this application will not include
 * dependencies outside of the lookahead range to be scheduled. This implies
 * that if a dependency, ```dep```, for  a blob, ```b```, is after the lookahead
 * range, then the dependency will ```dep``` will not be scheduled before
 * ```b```.
 *
 * However, the cost function will not add illegal schedule cost for missing
 * dependency(ies), returning the schedule as normal.
 *
 * Illegal schedules are intentionally given a large constant
 * (```ILLEGAL_SCHEDULE_COST```) instead of failing for two reasons:
 *
 * 1. Even if the schedule settles on an illegal schedule, there might be
 * *legal* schedules which the scheduler has yet to find, so failing early is
 * not the correct behavior.
 *
 * 2. Even if the schedule cannot find an illegal schedule, it should still
 * return a schedule, and the frontend/backend should handle this case
 * appropriately. It is not the responsibility for the scheduler to handle what
 * to do in the case of illegal schedules.
 *
 * ============================================================================
 * Overlap Cost
 * ============================================================================
 *
 * Overlap cost exists to ensure that the scheduler reduces the amount of
 * overlap between two overlappable blobs as much as possible. This is because
 * if free time exists on the calendar, the scheduler should try to utilize this
 * free time first before attempting to overlap blobs.
 *
 * ============================================================================
 * Split Cost
 * ============================================================================
 *
 * Split cost exists to ensure that the scheduler reduces the amount of
 * splitting that happens for a splittable blob. This is because lots of
 * splitting reduces efficiency and solution quality of the engine since more
 * blobs that need to be scheduled now exists within the lookahead range. The
 * hope is that implementing split cost encourages the scheduler to first look
 * for optima which splits as little as possible.
 *
 */

typedef uint32_t sec_t;

typedef struct interval {
  sec_t low;
  sec_t high;
} interval_t;

typedef struct tag tag_t;
typedef tag_t jid_t; /// an dependency ID is a Tag with an empty description
typedef struct policy policy_t;

typedef struct schedule schedule_t;
typedef struct jobs jobs_t;
typedef struct pair pair_t;

typedef struct container {
  size_t size;
  size_t capacity;
  void** data;
} container;

typedef container set;
typedef container vec;
typedef set tag_set_t;
typedef vec tag_vec_t;

typedef struct job {
  sec_t duration;
  interval_t schedulable_tr;
  interval_t scheduled_tr;
  jid_t *id;
  policy_t *policy;
  set *dependencies;
  set *tags;
} job_t;

pair_t *mk_pair(void *U, void *V);
void pair_free(pair_t *pair);

//=================================
//=========== CONSTANTS ===========
//=================================
static const sec_t MINUTE = (sec_t)60;
static const sec_t HOUR_TO_MINUTES = (sec_t)60;
static const sec_t DAY_TO_HOURS = (sec_t)24;
static const sec_t WEEK_TO_DAYS = (sec_t)7;

static const sec_t HOUR = ((sec_t)60 * MINUTE);
static const sec_t DAY = ((sec_t)24 * HOUR);
static const sec_t WEEK = ((sec_t)7 * DAY);

static const double ILLEGAL_SCHEDULE_COST = 1e12;

static const double EPSILON = 1e-8;
static const unsigned int DEFAULT_RNG_SEED = 1337;

//==========================================
//=========== INTERNAL CONSTANTS ===========
//==========================================

/**
 * @brief Internal constants
 *
 * These are constants used by elastisched's core scheduling
 * engine. However, they are intentionally exposed to the
 * user since these intrinsics might be useful in optimizing
 * scheduling calls.
 */
static const size_t ELASTISCHED_INITIAL_TAGCONTAINER_CAPACITY = 8;
static const size_t ELASTISCHED_INITIAL_CONTAINER_CAPACITY = 256;
static const size_t ELASTISCHED_INITIAL_MAP_CAPACITY = 32;
static const size_t ELASTISCHED_INTERNAL_DLL_SZ = 3 * sizeof(size_t);

// POLICY UTILS
/**
 * @brief creates a scheduling policy configuration struct
 *
 * @param is_splittable
 * @param is_overlappable
 * @param is_invisible
 * @param max_splits
 * @param min_split_duration
 * @return policy_t*
 */
policy_t *mk_policy(bool is_splittable, bool is_overlappable, bool is_invisible,
                    uint8_t max_splits, sec_t min_split_duration);
bool policy_is_splittable(policy_t *policy);
bool policy_is_overlappable(policy_t *policy);
bool policy_is_invisible(policy_t *policy);
uint8_t policy_max_splits(policy_t *policy);
sec_t min_split_duration(policy_t *policy);

schedule_t *mk_schedule();
void schedule_free(schedule_t *schedule);
void schedule_add_job(schedule_t *schedule, const job_t *job);
void schedule_clear(schedule_t *schedule);

/**
 * TAGS
 *
 * Definitions for working with the Tag and TagContainer structs.
 *
 * TagContainer is a simple container struct which allows for
 * functions to operate on it and implement Sets and Vectors.
 *
 * A set of tags in practice does not get very large nor updated
 * often so we opt for a simple vector-based sorted set.
 *
 * Two tags are equivalent if they share the same name,
 * not necessarily the same description.
 *
 * This allows for O(logn) search time and membership
 * checking, O(n) set difference and O(n) time to add
 * to the set.
 */

tag_t *mk_tag(char *name, char *description);
void tag_free(tag_t *tag);
bool tag_eq(const tag_t *U, const tag_t *V);
int tag_cmp(const tag_t *U, const tag_t *V);
uint64_t tag_hash(const tag_t *U);

tag_set_t *mk_tag_container(size_t capacity);
void tag_container_free(container *container);

/**
 * @brief Helper function for adding tag into a set
 *
 * @param set
 * @param tag
 * @return true if no memory failures, false otherwise
 */
bool ts_add(tag_set_t *set, tag_t tag);

/**
 * @brief Membership checking in O(log|```set```|)
 *
 * @param set
 * @param tag
 * @return true if ```tag``` is in ```set```
 */
bool ts_in(tag_set_t *set, tag_t tag);

/**
 * @brief returns a new set containing the set union
 *
 * @param U
 * @param V
 * @return TagContainer*
 *
 * @note runtime of this is O(|U| + |V|)
 */
tag_set_t *ts_union(tag_set_t *U, tag_set_t *V);

/**
 * @brief returns a new set containing the set intersection
 *
 * @param U
 * @param V
 * @return TagContainer*
 *
 * @note runtime of this is O(min(|U|, |V|)*log(max(|U|, |V|)))
 */
tag_set_t *ts_intersection(tag_set_t *U, tag_set_t *V);

/**
 * @brief ensures that the tag set internals meet the appropriate
 * invariants.
 *
 * @param set
 * @return: true if the invariants are met
 */
bool ts_is_valid(tag_set_t *set);

/**
 * @brief pushback tag to tagvec
 *
 * @param vec
 * @param tag
 * @return: true if no memory failures, false otherwise
 */
bool tv_pushback(tag_vec_t *vec, tag_t tag);

schedule_t *generate_random_schedule_neighbor(schedule_t *schedule,
                                              sec_t granularity,
                                              unsigned int seed);
pair_t *_schedule_jobs(jobs_t jobs, sec_t granularity, double initial_temp,
                       double final_temp, uint64_t num_iters);
pair_t *schedule_jobs(jobs_t jobs, sec_t granularity);

#endif