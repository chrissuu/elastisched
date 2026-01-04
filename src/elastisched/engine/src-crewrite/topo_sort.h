
#include "constants.h"
#include "dll.h"
#include "tag.h"
#include "utils.h"

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

