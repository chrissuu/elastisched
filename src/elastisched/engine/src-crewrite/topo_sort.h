#ifndef ELASTISCHED_TOPO_SORT_H
#define ELASTISCHED_TOPO_SORT_H

#include <stdbool.h>
#include <stddef.h>
#include "constants.h"
#include "dll.h"
#include "map.h"
#include "tag.h"
#include "utils.h"
#include "schedule.h"

typedef struct DependencyViolation {
    ID job_id;
    DependencyContainer* violated_dependencies;
} DependencyViolation;

typedef struct DependencyViolationContainer {
    DependencyViolation* dependency_violations;
    size_t size;
    size_t capacity;
} DependencyViolationContainer;

typedef struct DependencyCheckResult {
    bool has_violations;
    DependencyViolationContainer* violations;
    bool has_cyclic_dependencies;
} DependencyCheckResult;

DependencyCheckResult* check_dependency_violations(const Schedule* schedule);

#endif
