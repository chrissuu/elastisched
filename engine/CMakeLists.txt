cmake_minimum_required(VERSION 3.16)
project(ElastischedSchedulerEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
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
    src/Job.cpp
    src/Policy.cpp
    src/engine.cpp
    src/Tag.cpp
)

target_include_directories(scheduler_lib PUBLIC 
    src
)

option(ELASTISCHED_BUILD_CLI "Build the engine CLI executable" ON)
if(NOT SKBUILD AND ELASTISCHED_BUILD_CLI)
    set(CMAKE_BINARY_DIR "..")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")
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
    else()
        message(STATUS "Skipping CLI build: src/main.cpp not found.")
        install(TARGETS scheduler_lib
                LIBRARY DESTINATION lib
                ARCHIVE DESTINATION lib)
    endif()
elseif(NOT SKBUILD)
    install(TARGETS scheduler_lib
            LIBRARY DESTINATION lib
            ARCHIVE DESTINATION lib)
endif()

option(ELASTISCHED_BUILD_TESTS "Build engine C++ tests" ON)
if(ELASTISCHED_BUILD_TESTS)
    enable_testing()
    add_executable(engine_tests
        tests/tests.cpp
    )
    target_link_libraries(engine_tests PRIVATE scheduler_lib)
    target_include_directories(engine_tests PRIVATE
        src
        tests
    )
    add_test(NAME engine_tests COMMAND engine_tests)
endif()

option(ELASTISCHED_ENABLE_COVERAGE "Enable coverage instrumentation" OFF)
if(ELASTISCHED_ENABLE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(scheduler_lib PRIVATE -O0 -g --coverage)
        target_link_options(scheduler_lib PRIVATE --coverage)
        if(ELASTISCHED_BUILD_TESTS)
            target_compile_options(engine_tests PRIVATE -O0 -g --coverage)
            target_link_options(engine_tests PRIVATE --coverage)
        endif()
    endif()
endif()

if(pybind11_FOUND AND SKBUILD)
    pybind11_add_module(engine_py 
        src/pybind_scheduler.cpp
    )
    
    target_link_libraries(engine_py PRIVATE scheduler_lib)
    
    set_target_properties(engine_py PROPERTIES 
        OUTPUT_NAME "engine"
    )
    
    target_include_directories(engine_py PRIVATE
        src
    )
    
    target_compile_definitions(engine_py PRIVATE VERSION_INFO=${PROJECT_VERSION})
    
    install(TARGETS engine_py DESTINATION .)

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
