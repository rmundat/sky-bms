# Specify the minimum version for CMake
cmake_minimum_required(VERSION 3.10)

# Project name 
project(sky-bms)

# -g allows for gdb debugging
# Turn on -O3 for best performance
add_definitions(-std=c++17 -O3)

set(CXX_FLAGS "-Wall")
set(CMAKE_CXX_FLAGS "${CXX_FLAGS}")
# Adding -pg flag for gprof profiling
add_compile_options(-pg)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")

# Include root directory
include_directories(${CMAKE_SOURCE_DIR})

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/modules)

# Source files
file(GLOB SOURCES 
    "${CMAKE_SOURCE_DIR}/modules/battery/src/*.cpp" 
    "${CMAKE_SOURCE_DIR}/modules/bms/src/*.cpp" 
    "${CMAKE_SOURCE_DIR}/modules/data/*.cpp" 
    "${CMAKE_SOURCE_DIR}/modules/utils/*.cpp" 
    "${CMAKE_SOURCE_DIR}/modules/common/constants.h" 
    "${CMAKE_SOURCE_DIR}/modules/main.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})

# Setting target directory to source root
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})