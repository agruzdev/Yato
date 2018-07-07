#
# YATO library
#
# The MIT License (MIT)
# Copyright (c) 2018 Alexey Gruzdev
#
#
# Google benchmark dependency
# https://github.com/google/benchmark
#
# Output variables:
# GBENCH_INCLUDE_DIR - includes
# GBENCH_LIBRARY_DIR - link directories
# GBENCH_LIBRARIES   - link targets

# Depends on Gtest
include(${YATO_SOURCE_DIR}/cmake/dependency.gtest.cmake)

dependency_find_or_download(
    NAME GBENCH
    VERBOSE_NAME "GoogleBenchmark"
    URL "https://github.com/google/benchmark/archive/v1.4.1.zip"
    HASH_MD5 "619674faa0d878e239eaf6766259718b"
    PREFIX "benchmark-1.4.1"
)

if(NOT TARGET benchmark)
    set(GTEST_LIBRARY $<TARGET_FILE:gtest>)
    set(GTEST_MAIN_LIBRARY $<TARGET_FILE:gtest_main>)
    set(GMOCK_INCLUDE_DIRS ${GMOCK_INCLUDE_DIR})

    set(BENCHMARK_ENABLE_TESTING OFF)
    set(HAVE_STD_REGEX ON)

    add_subdirectory(${GBENCH_FOUND_ROOT} ${CMAKE_BINARY_DIR}/dependency/gbench)

    set_property(TARGET benchmark      PROPERTY FOLDER "Dependencies")
    set_property(TARGET benchmark_main PROPERTY FOLDER "Dependencies")
endif()

set(GBENCH_INCLUDE_DIR ${GBENCH_FOUND_ROOT}/include CACHE INTERNAL "")
set(GBENCH_LIBRARY_DIR "" CACHE INTERNAL "")
list(APPEND GBENCH_LIBRARIES benchmark)
list(APPEND GBENCH_LIBRARIES benchmark_main)
set(GBENCH_LIBRARIES ${GBENCH_LIBRARIES} CACHE INTERNAL "")


