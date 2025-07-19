#include "schedule.hpp"

void Schedule::addJob(const Job& job) {
    scheduledJobs.push_back(job);
}

void Schedule::clear() {
    scheduledJobs.clear();
}

std::ostream& operator<<(std::ostream& os, const Schedule& schedule) {
    const auto& jobs = schedule.scheduledJobs;
    os << "Schedule contains " << jobs.size() << " job(s):\n";
    for (const auto& job : jobs) {
        os << "  - Job Name: " << job.id << ", Scheduled Time: " << job.scheduledTimeRange << "\n";
    }
    return os;
}