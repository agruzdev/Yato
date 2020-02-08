#
# YATO library
#
# Apache License, Version 2.0
# Copyright (c) 2016-2020 Alexey Gruzdev
#

cmake_minimum_required (VERSION 3.8)

if(NOT YATO_NO_PROJECT_FOLDERS)
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
endif()

set(YATO_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

project(Yato)

#==========================================
# Output

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${BIN_OUTPUT_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${BIN_OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BIN_OUTPUT_DIR})

#==========================================
# Determine target

if(MSVC)
    set(TARGET_CONFIG_LISTS ${YATO_SOURCE_DIR}/cmake/CMakeLists-MSVC.cmake)
else()
    set(TARGET_CONFIG_LISTS ${YATO_SOURCE_DIR}/cmake/CMakeLists-GCC.cmake)
endif()

#==========================================
# Core sources

set(YATO_INCLUDE_DIRS "${YATO_SOURCE_DIR}/include")

file(GLOB_RECURSE yato_sources "${YATO_INCLUDE_DIRS}/*.h")

source_group(TREE "${YATO_SOURCE_DIR}" FILES ${yato_sources})

set(yato_extra_sources "")
if(MSVC)
    list(APPEND yato_extra_sources ${YATO_SOURCE_DIR}/extras/yato.natvis)
endif()

add_custom_target(Yato SOURCES ${yato_sources}
                               ${yato_extra_sources}
)
set_property(TARGET Yato PROPERTY FOLDER "Modules")

#==========================================
# Targets

list(APPEND Yato_TEST_DIRS) # List of all test directories
list(APPEND Yato_TEST_LIBS) # List of all test dependensies

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()
message(STATUS "CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}")

option(YATO_BUILD_CONFIG "Build config module" OFF)
if(YATO_BUILD_CONFIG)
    add_subdirectory(./modules/config)
    list(APPEND YATO_INCLUDE_DIRS "${YATO_SOURCE_DIR}/modules/config")
endif()

option(YATO_BUILD_ACTORS "Build actors module" OFF)
if(YATO_BUILD_ACTORS)
    option(YATO_ACTORS_WITH_IO "Enable networking component" OFF)

    add_subdirectory(./modules/actors)
    list(APPEND YATO_INCLUDE_DIRS "${YATO_SOURCE_DIR}/modules/actors")
endif()

#==========================================
# Tests

option(YATO_BUILD_TESTS "Build unit tests" ON)
if(YATO_BUILD_TESTS)
    add_subdirectory(./tests)
endif()

option(YATO_BUILD_PERFORMANCE_TESTS "Buidl performance tests" OFF)
if(YATO_BUILD_PERFORMANCE_TESTS)
    add_subdirectory(./perf_tests)
endif()

