cmake_minimum_required(VERSION 3.16)
project(JobScheduler)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -O2")
set(CMAKE_BINARY_DIR "..")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_library(scheduler_lib
    src/job.cpp
    src/policy.cpp
    src/schedule.cpp
    src/cost_function.cpp
    src/tag.cpp
)
target_include_directories(scheduler_lib PUBLIC src src/utils src/optimizer)

add_executable(scheduler src/scheduler.cpp)
target_link_libraries(scheduler scheduler_lib)

set_target_properties(scheduler PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    OUTPUT_NAME "job_scheduler"  # Optional: rename the executable
)

install(TARGETS scheduler scheduler_lib
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)