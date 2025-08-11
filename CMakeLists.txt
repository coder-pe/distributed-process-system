cmake_minimum_required(VERSION 3.10)
project(DistributedProcessingSystem VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 98)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(include)

# Find required packages
find_package(Threads REQUIRED)

# Source files
file(GLOB SOURCES "src/*.cpp")
list(REMOVE_ITEM SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")

# Create library
add_library(distributed_system STATIC ${SOURCES})

# Main executable
add_executable(distributed_system_main src/main.cpp)
target_link_libraries(distributed_system_main distributed_system Threads::Threads dl rt)

# Tests
file(GLOB TEST_SOURCES "tests/*.cpp")
foreach(test_source ${TEST_SOURCES})
    get_filename_component(test_name ${test_source} NAME_WE)
    add_executable(${test_name} ${test_source})
    target_link_libraries(${test_name} distributed_system Threads::Threads dl rt)
endforeach()

# Examples
file(GLOB EXAMPLE_DIRS "examples/*")
foreach(example_dir ${EXAMPLE_DIRS})
    if(IS_DIRECTORY ${example_dir})
        get_filename_component(example_name ${example_dir} NAME)
        file(GLOB example_sources "${example_dir}/*.cpp")
        if(example_sources)
            add_executable(example_${example_name} ${example_sources})
            target_link_libraries(example_${example_name} distributed_system Threads::Threads dl rt)
        endif()
    endif()
endforeach()
