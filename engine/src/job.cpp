#include "job.hpp"

Job::Job(time_t duration, TimeRange schedulableTimeRange, TimeRange scheduledTimeRange,
        ID id, Policy policy, std::set<ID> dependencies, std::set<Tag> tags) 
:   duration(duration),
    schedulableTimeRange(schedulableTimeRange),
    scheduledTimeRange(scheduledTimeRange),
    scheduledTimeRanges({scheduledTimeRange}),
    id(id),
    policy(policy),
    dependencies(dependencies),
    tags(tags)
{
        return;
};

bool Job::isRigid() const {
    return duration == schedulableTimeRange.length();
};

const std::vector<TimeRange>& Job::getScheduledTimeRanges() const {
    return scheduledTimeRanges;
}

void Job::setScheduledTimeRanges(std::vector<TimeRange> ranges) {
    scheduledTimeRanges = std::move(ranges);
    if (!scheduledTimeRanges.empty()) {
        scheduledTimeRange = scheduledTimeRanges.front();
    }
}

std::string Job::toString() const {
    std::ostringstream oss;
    
    oss << "Job(id=" << id << ")\n";
    oss << "├─ Duration: " << duration << " seconds\n";
    
    // Format schedulable time range
    oss << "├─ Schedulable: [" << schedulableTimeRange.getLow() 
        << " - " << schedulableTimeRange.getHigh() << "]";
    if (schedulableTimeRange.length() > 0) {
        oss << " (length: " << schedulableTimeRange.length() << "s)";
    }
    oss << "\n";
    
    // Format scheduled time range
    oss << "├─ Scheduled: ";
    if (!scheduledTimeRanges.empty() || scheduledTimeRange.length() > 0) {
        const auto& primary = scheduledTimeRanges.empty() ? scheduledTimeRange : scheduledTimeRanges.front();
        oss << "[" << primary.getLow() 
            << " - " << primary.getHigh() << "]";
        oss << " (length: " << primary.length() << "s)";
        if (scheduledTimeRanges.size() > 1) {
            oss << " (split segments: " << scheduledTimeRanges.size() << ")";
        }
    } else {
        oss << "Not scheduled";
    }
    oss << "\n";
    
    // Policy information
    oss << "├─ Policy: ";
    if (policy.isSplittable()) {
        oss << "Splittable (max: " << static_cast<int>(policy.getMaxSplits()) 
            << ", min duration: " << policy.getMinSplitDuration() << "s)";
    } else {
        oss << "Non-splittable";
    }
    if (policy.isOverlappable()) {
        oss << ", Overlappable";
    }
    oss << "\n";
    
    // Dependencies
    oss << "├─ Dependencies: ";
    if (dependencies.empty()) {
        oss << "None";
    } else {
        oss << "[";
        bool first = true;
        for (const auto& dep : dependencies) {
            if (!first) oss << ", ";
            oss << dep;
            first = false;
        }
        oss << "]";
    }
    oss << "\n";
    
    // Tags
    oss << "└─ Tags: ";
    if (tags.empty()) {
        oss << "None";
    } else {
        oss << "[";
        bool first = true;
        for (const auto& tag : tags) {
            if (!first) oss << ", ";
            oss << tag.getName();
            first = false;
        }
        oss << "]";
    }
    
    return oss.str();
}
