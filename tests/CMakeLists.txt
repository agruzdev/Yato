project(YatoTests)

#==========================================
# Dependencies

set(GTEST_ROOT "$ENV{GTEST_ROOT}" CACHE PATH "Path to google-test dir with cmake lists")

add_subdirectory(${GTEST_ROOT} ${CMAKE_BINARY_DIR}/gtest)
set(GTEST_INCLUDE_DIR "${GTEST_ROOT}/googletest/include" CACHE PATH "Path to includes")
message(STATUS "gtest include = ${GTEST_INCLUDE_DIR}")


include(${TARGET_CONFIG_LISTS})

#==========================================
# Sources

include_directories(${GTEST_INCLUDE_DIR})
include_directories("${CMAKE_SOURCE_DIR}/include")

file(GLOB_RECURSE all_tests "source/*.cpp" "source/*.h")
file(GLOB_RECURSE yato_sources "${CMAKE_SOURCE_DIR}/include/*.h")
list(APPEND all_sources ${all_tests})
list(APPEND all_sources ${yato_sources})

#==========================================
# Targets

add_executable(YatoTests ${all_sources})

target_link_libraries(YatoTests gtest)
target_link_libraries(YatoTests gtest_main)