#include <vector>

class Schedule {
private:
    std::vector<ScheduledJob> scheduled_jobs;
    std::unordered_map<JobId, size_t> job_to_index;
    
public:
    void add_scheduled_job(const ScheduledJob& job) {
        job_to_index[job.job_id] = scheduled_jobs.size();
        scheduled_jobs.push_back(job);
    }
    
    const ScheduledJob* get_scheduled_job(JobId job_id) const {
        auto it = job_to_index.find(job_id);
        if (it != job_to_index.end()) {
            return &scheduled_jobs[it->second];
        }
        return nullptr;
    }
    
    const std::vector<ScheduledJob>& get_all_jobs() const {
        return scheduled_jobs;
    }
    
    TimeUnit get_makespan() const {
        TimeUnit max_end = 0;
        for (const auto& job : scheduled_jobs) {
            max_end = std::max(max_end, job.end_time);
        }
        return max_end;
    }
    
    bool is_valid() const {
        // TODO: Implement validation logic
        // - Check precedence constraints
        // - Check resource conflicts
        // - Check time windows
        return true;
    }
};
