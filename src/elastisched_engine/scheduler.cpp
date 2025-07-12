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

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <memory>
#include <algorithm>
#include <limits>
#include <queue>

using BlobId = int;

struct Blob {
    BlobId id;
    TimeRange schedulable_timerange;
    std::unordered_set<BlobId> dependencies;
};
class Blob {
public:
    BlobId id;
    TimeUnit duration;
    TimeUnit earliest_start;
    TimeUnit latest_finish;
    std::vector<ResourceId> required_resources;
    std::unordered_set<JobId> dependencies;  // Jobs that must complete before this one
    std::unordered_map<std::string, double> properties;  // Custom job properties
    
    Job(JobId job_id, TimeUnit dur) 
        : id(job_id), duration(dur), earliest_start(0), 
          latest_finish(std::numeric_limits<TimeUnit>::max()) {}
    
    void add_dependency(JobId dep_job) {
        dependencies.insert(dep_job);
    }
    
    void set_time_window(TimeUnit earliest, TimeUnit latest) {
        earliest_start = earliest;
        latest_finish = latest;
    }
    
    void add_resource_requirement(ResourceId resource) {
        required_resources.push_back(resource);
    }
    
    void set_property(const std::string& key, double value) {
        properties[key] = value;
    }
}