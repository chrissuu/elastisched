#ifndef DEPENDENCYCHECKER_H
#define DEPENDENCYCHECKER_H

#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <vector>
#include <iostream>
#include <set>

#include "../constants.hpp"
#include "../schedule.hpp"

struct DependencyViolation {
    ID jobId;
    std::set<ID> violatedDependencies; // Dependencies that haven't been scheduled before this job
    
    DependencyViolation(ID id, const std::set<ID>& violations) 
        : jobId(id), violatedDependencies(violations) {}
};

struct DependencyCheckResult {
    bool hasViolations;
    std::vector<DependencyViolation> violations;
    bool hasCyclicDependencies;
    
    DependencyCheckResult() : hasViolations(false), hasCyclicDependencies(false) {}
};


/**
 * checkDependencyViolations
 * 
 * ->Topologically sorts dependency graph
 * ->Iterates through scheduled jobs and ensures that for each dependency of the job, low() appears
 *   earlier than the scheduled job
 */
inline DependencyCheckResult checkDependencyViolations(const Schedule& schedule) {
    DependencyCheckResult result;
    
    if (schedule.scheduledJobs.empty()) {
        return result;
    }
    
    std::unordered_map<ID, const Job*> jobMap;
    for (const auto& job : schedule.scheduledJobs) {
        jobMap[job.id] = &job;
    }
    
    std::unordered_map<ID, std::vector<ID>> adjList;
    std::unordered_map<ID, int> inDegree;
    
    for (const auto& job : schedule.scheduledJobs) {
        inDegree[job.id] = 0;
        adjList[job.id] = std::vector<ID>();
    }
    
    for (const auto& job : schedule.scheduledJobs) {
        for (const ID& depId : job.dependencies) {
            if (jobMap.find(depId) != jobMap.end()) {
                adjList[depId].push_back(job.id);
                inDegree[job.id]++;
            }
        }
    }
    
    std::queue<ID> queue;
    std::vector<ID> topologicalOrder;
    
    for (const auto& pair : inDegree) {
        if (pair.second == 0) {
            queue.push(pair.first);
        }
    }
    
    while (!queue.empty()) {
        ID currentId = queue.front();
        queue.pop();
        topologicalOrder.push_back(currentId);
        
        for (ID neighborId : adjList[currentId]) {
            inDegree[neighborId]--;
            if (inDegree[neighborId] == 0) {
                queue.push(neighborId);
            }
        }
    }
    
    if (topologicalOrder.size() != schedule.scheduledJobs.size()) {
        result.hasCyclicDependencies = true;
        result.hasViolations = true;
        return result;
    }

    std::unordered_set<ID> processedJobs;
    
    for (const auto& job : schedule.scheduledJobs) {
        std::set<ID> violatedDeps;
        
        for (const ID& depId : job.dependencies) {
            if (jobMap.find(depId) != jobMap.end() && 
                processedJobs.find(depId) == processedJobs.end()) {
                violatedDeps.insert(depId);
            }
        }
        
        if (!violatedDeps.empty()) {
            result.violations.emplace_back(job.id, violatedDeps);
            result.hasViolations = true;
        }
        
        processedJobs.insert(job.id);
    }
    
    return result;
}

#endif