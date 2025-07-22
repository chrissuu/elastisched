cmake_minimum_required(VERSION 3.16)
project(ElastischedSchedulerEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS "-Wall -Wextra -O0 -g")
else()
    set(CMAKE_CXX_FLAGS "-Wall -Wextra -O2")
endif()

find_package(pybind11 QUIET)

# Core scheduler library (shared between executable and Python module)
add_library(scheduler_lib
    src/job.cpp
    src/policy.cpp
    src/schedule.cpp
    src/cost_function.cpp
    src/tag.cpp
)

target_include_directories(scheduler_lib PUBLIC 
    src 
    src/utils 
    src/optimizer
)

if(NOT SKBUILD)
    set(CMAKE_BINARY_DIR "..")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    
    add_executable(scheduler src/main.cpp)
    target_link_libraries(scheduler scheduler_lib)
    
    set_target_properties(scheduler PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        OUTPUT_NAME "engine"
    )
    
    install(TARGETS scheduler scheduler_lib
            RUNTIME DESTINATION bin
            LIBRARY DESTINATION lib
            ARCHIVE DESTINATION lib)
endif()

# Python extension module (only build if pybind11 is found and SKBUILD is set)
if(pybind11_FOUND AND SKBUILD)
    pybind11_add_module(engine_py 
        src/pybind_scheduler.cpp     # Your pybind file
        # Don't include main.cpp here since it has a main() function
    )
    
    # Link against the core library
    target_link_libraries(engine_py PRIVATE scheduler_lib)
    
    # Set the module name to match your Python import
    set_target_properties(engine_py PROPERTIES 
        OUTPUT_NAME "engine"  # This will be the Python module name
    )
    
    # Include directories for the Python module
    target_include_directories(engine_py PRIVATE
        src
        src/utils
        src/optimizer
    )
    
    # Set properties for the module
    target_compile_definitions(engine_py PRIVATE VERSION_INFO=${PROJECT_VERSION})
    
    # Install the Python module (scikit-build-core handles this)
    install(TARGETS engine_py DESTINATION .)
    
    # Platform-specific settings for Python module
    if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        set_target_properties(engine_py PROPERTIES
            INSTALL_RPATH "@loader_path"
        )
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        set_target_properties(engine_py PROPERTIES
            INSTALL_RPATH "$ORIGIN"
        )
    endif()
endif()