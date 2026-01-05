
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
} DependencyViolationContainer;

typedef struct DependencyCheckResult {
    bool has_violations;
    DependencyViolationContainer* violations;
    bool has_cyclic_dependencies;
} DependencyCheckResult;

DependencyCheckResult* check_dependency_violations(const Schedule* schedule);

