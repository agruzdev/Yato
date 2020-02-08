#
# YATO library
#
# Apache License, Version 2.0
# Copyright (c) 2016-2020 Alexey Gruzdev
#

project(YatoTests)

#==========================================
# Dependencies

include(${YATO_SOURCE_DIR}/cmake/dependency.gtest.cmake)

include(${TARGET_CONFIG_LISTS})

#==========================================
# Sources

include_directories(${GTEST_INCLUDE_DIR})
include_directories(${YATO_INCLUDE_DIRS})

file(GLOB all_tests "${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/source/*.h")
list(APPEND all_sources ${all_tests})

foreach(test_dir ${Yato_TEST_DIRS})
    message(STATUS "Added tests from: ${test_dir}")
    file(GLOB test_files "${test_dir}/*.cpp" "${test_dir}/*.h")
    list(APPEND all_sources ${test_files})
endforeach()

source_group(TREE "${YATO_SOURCE_DIR}" FILES ${all_sources})

#==========================================
# Targets

add_definitions(-DYATO_ENABLE_TESTED_ASSERTIONS)
link_directories(${GTEST_LIBRARY_DIR})

add_executable(YatoTests ${all_sources})

set_property(TARGET YatoTests PROPERTY FOLDER "Tests")

# Quickfix of gtest bugs
_gtest_fix_definitions(YatoTests)

foreach(lib ${Yato_TEST_LIBS})
    target_link_libraries(YatoTests ${lib})
endforeach()
foreach(lib ${GTEST_LIBRARIES})
    target_link_libraries(YatoTests ${lib})
endforeach()
