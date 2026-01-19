#ifndef ENGINE_HPP
#define ENGINE_HPP

#include "constants.hpp"
#include "types.hpp"
#include "Job.hpp"
#include "IntervalTree.hpp"

#include <optional>
#include <set>
#include <utility>
#include <vector>
#include <ostream>

class Schedule {
public:
    std::vector<Job> scheduled_jobs;

    Schedule() = default;
    Schedule(std::vector<Job> scheduled_jobs);
    void add_job(const Job& job);
    void clear();

private:
    friend std::ostream& operator<<(std::ostream& os, const Schedule& schedule);
};

std::ostream& operator<<(std::ostream& os, const Schedule& schedule);

struct DependencyViolation {
    ID job_id;
    std::set<ID> violated_dependencies; // Dependencies that haven't been scheduled before this job

    DependencyViolation(ID job_id, const std::set<ID>& violated_dependencies);
};

struct DependencyCheckResult {
    bool has_violations;
    std::vector<DependencyViolation> violations;
    bool has_cyclic_dependencies;

    DependencyCheckResult();
};

DependencyCheckResult check_dependency_violations(const Schedule& schedule);

class ScheduleCostFunction {
private:
    const Schedule& schedule_ref;
    const sec_t granularity;
    const std::set<Tag> rest_tags{};
    IntervalTree<sec_t, std::optional<std::vector<Job>>> day_based_schedule;
    std::optional<sec_t> min_time = std::nullopt;
    std::optional<sec_t> max_time = std::nullopt;

public:
    double context_switch_cost() const;
    double illegal_schedule_cost() const;
    double overlap_cost() const;
    double split_cost() const;
    double schedule_cost() const;

    ScheduleCostFunction(const Schedule& schedule, sec_t granularity);
};

Schedule schedule(std::vector<Job> jobs, const uint64_t granularity);
std::pair<Schedule, std::vector<double>> schedule_jobs(
    std::vector<Job> jobs,
    const uint64_t granularity,
    const double initial_temp,
    const double final_temp,
    const uint64_t num_iters);

#endif // ENGINE_HPP
