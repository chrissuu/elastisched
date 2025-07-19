#include "job.hpp"
#include "policy.hpp"
#include "schedule.hpp"
#include "utils/IntervalTree.hpp"
#include "cost_function.hpp"
#include "tests.hpp"

#include "constants.hpp"

#include <vector>
#include <memory>
#include <algorithm>
#include <cstddef>
#include <map>
#include <random>
#include <optional>
#include <iostream>
#include <set>
#include <cassert>

#include "optimizer/SimulatedAnnealingOptimizer.hpp"

int main() {
    std::cout << "=== Scheduler Test Suite ===\n\n";
    
    try {
        test_empty_schedule();
        test_basic_scheduling();
        test_rigid_job();
        test_friday_cost_optimization();
        
        std::cout << "=== All tests passed! ===\n";
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Test failed with unknown exception\n";
        return 1;
    }
    
    return 0;
}
