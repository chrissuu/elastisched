#ifndef SCHEDULE_HPP
#define SCHEDULE_HPP

#include <vector>
#include "job.hpp"

class Schedule {
public:
    std::vector<Job> scheduledJobs;

    Schedule() = default;
    void addJob(const Job& job);
    void clear();

private:
    friend std::ostream& operator<<(std::ostream& os, const Schedule& schedule);
};

#endif // SCHEDULE_HPP
