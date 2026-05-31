#
# YATO library
#
# Apache License, Version 2.0
# Copyright (c) 2016-2020 Alexey Gruzdev
#

cmake_minimum_required (VERSION 3.15)

if(NOT YATO_NO_PROJECT_FOLDERS)
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
endif()

set(YATO_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

project(Yato VERSION 1.2.0 LANGUAGES CXX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

#==========================================
# Output

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${BIN_OUTPUT_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${BIN_OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BIN_OUTPUT_DIR})

#==========================================
# Determine target

set(YATO_CXX_STANDARD "c++20" CACHE STRING "C++ standard version")
set_property(CACHE YATO_CXX_STANDARD PROPERTY STRINGS "c++20" "c++17" "c++14")

if(MSVC)
    include(${YATO_SOURCE_DIR}/cmake/CMakeLists-MSVC.cmake)
else()
    include(${YATO_SOURCE_DIR}/cmake/CMakeLists-GCC.cmake)
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


add_library(Yato INTERFACE)

set_property(TARGET Yato PROPERTY FOLDER "Modules")

target_sources(Yato INTERFACE
    FILE_SET HEADERS
    BASE_DIRS ${YATO_SOURCE_DIR}/include
    FILES ${yato_sources}
)

target_include_directories(Yato INTERFACE
    $<BUILD_INTERFACE:${YATO_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

#==========================================
# Installation

# Generate and install CMake config files.
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/YatoConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)


configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/YatoConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/YatoConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Yato
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/YatoConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/YatoConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Yato
)

# Install header-only library.
install(TARGETS Yato
    EXPORT YatoTargets
    FILE_SET HEADERS
)

install(EXPORT YatoTargets
    FILE YatoTargets.cmake
    NAMESPACE Yato::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Yato
)

#==========================================
# Tests

option(YATO_BUILD_TESTS "Build unit tests" ON)
if(YATO_BUILD_TESTS)
    enable_testing()
    add_subdirectory(./tests)
endif()

option(YATO_BUILD_PERFORMANCE_TESTS "Build performance tests" OFF)
if(YATO_BUILD_PERFORMANCE_TESTS)
    add_subdirectory(./perf_tests)
endif()

