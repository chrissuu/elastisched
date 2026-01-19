#ifndef ENGINE_HPP
#define ENGINE_HPP

#include "constants.hpp"
#include "job.hpp"
#include "IntervalTree.hpp"

#include <optional>
#include <set>
#include <utility>
#include <vector>
#include <ostream>

class Schedule {
public:
    std::vector<Job> scheduledJobs;

    Schedule() = default;
    Schedule(std::vector<Job> scheduledJobs);
    void addJob(const Job& job);
    void clear();

private:
    friend std::ostream& operator<<(std::ostream& os, const Schedule& schedule);
};

std::ostream& operator<<(std::ostream& os, const Schedule& schedule);

struct DependencyViolation {
    ID jobId;
    std::set<ID> violatedDependencies; // Dependencies that haven't been scheduled before this job

    DependencyViolation(ID id, const std::set<ID>& violations);
};

struct DependencyCheckResult {
    bool hasViolations;
    std::vector<DependencyViolation> violations;
    bool hasCyclicDependencies;

    DependencyCheckResult();
};

DependencyCheckResult checkDependencyViolations(const Schedule& schedule);

class ScheduleCostFunction {
private:
    const Schedule& m_schedule;
    const time_t m_granularity;
    const std::set<Tag> m_restTags;
    IntervalTree<time_t, std::optional<std::vector<Job>>> m_dayBasedSchedule;
    std::optional<time_t> m_min = std::nullopt;
    std::optional<time_t> m_max = std::nullopt;

public:
    double context_switch_cost() const;
    double illegal_schedule_cost() const;
    double overlap_cost() const;
    double split_cost() const;
    double scheduleCost() const;

    ScheduleCostFunction(const Schedule& schedule, time_t granularity);
};

Schedule schedule(std::vector<Job> jobs, const uint64_t GRANULARITY);
std::pair<Schedule, std::vector<double>> scheduleJobs(
    std::vector<Job> jobs,
    const uint64_t GRANULARITY,
    const double INITIAL_TEMP,
    const double FINAL_TEMP,
    const uint64_t NUM_ITERS);

#endif // ENGINE_HPP
