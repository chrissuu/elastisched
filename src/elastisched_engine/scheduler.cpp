/**
 * Scheduler.cpp
 * 
 * The primary scheduling engine.
 * 
 * The scheduling engine is left intentionally abstract.
 * 
 * The problem is very similar to the optimal job scheduling problem/construction
 * for CPUs. There might exist a reduction to an existing discrete optimization solver
 * but I was sort of lazy to write/look for one, so I implemented my own.
 * 
 * Specifically, the following properties makes this scheduling problem difficult:
 * 
 *      -> Custom cost function(s)
 *          -> Multi-cost optimization
 *      -> Precedence relations
 * 
 * Custom cost functions can be defined to measure the "goodness" of a schedule.
 * 
 * The scheduling engine attempts to return the schedule that optimizes this cost 
 * as best as possible.
 */

#include <pybind11/pybind11.h>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <memory>
#include <algorithm>
#include <limits>
#include <queue>

struct Blob {
    vector
};