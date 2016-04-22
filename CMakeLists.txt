cmake_minimum_required (VERSION 3.1)

project(YatoTests)

#==========================================
# Dependencies

set(GTEST_ROOT "$ENV{GTEST_ROOT}" CACHE PATH "Path to google-test dir with cmake lists")

add_subdirectory(${GTEST_ROOT} ${CMAKE_BINARY_DIR}/gtest)
set(GTEST_INCLUDE_DIR "${GTEST_ROOT}/googletest/include" CACHE PATH "Path to includes")
message(STATUS "gtest include = ${GTEST_INCLUDE_DIR}")

#==========================================
# Output

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${BIN_OUTPUT_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${BIN_OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BIN_OUTPUT_DIR})

#==========================================
# Determine target

if(MSVC)
    include(./cmake/CMakeLists-MSVC.cmake)
elseif(UNIX OR MINGW OR ANDROID)
    include(./cmake/CMakeLists-GCC.cmake)
endif()

#==========================================
# Sources

include_directories(${GTEST_INCLUDE_DIR})
include_directories("${CMAKE_SOURCE_DIR}/include")

file(GLOB_RECURSE all_tests "${CMAKE_SOURCE_DIR}/tests/*.cpp" "${CMAKE_SOURCE_DIR}/tests/*.h")
file(GLOB_RECURSE yato_sources "${CMAKE_SOURCE_DIR}/include/*.h")
list(APPEND all_sources ${all_tests})
list(APPEND all_sources ${yato_sources})

#==========================================
# Targets

add_executable(YatoTests ${all_sources})

target_link_libraries(YatoTests gtest)
target_link_libraries(YatoTests gtest_main)