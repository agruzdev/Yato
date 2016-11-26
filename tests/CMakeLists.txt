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
list(APPEND all_sources ${all_tests})
list(APPEND all_sources ${yato_sources})

foreach(f ${all_sources})
    # Get the path of the file relative to ${CMAKE_HOME_DIRECTORY},
    # then alter it (not compulsory)
    file(RELATIVE_PATH SRCGR ${CMAKE_HOME_DIRECTORY} ${f})
    set(SRCGR "${SRCGR}")

    # Extract the folder, ie remove the filename part
    string(REGEX REPLACE "(.*)(/[^/]*)$" "\\1" SRCGR ${SRCGR})

    # Source_group expects \\ (double antislash), not / (slash)
    string(REPLACE / \\ SRCGR ${SRCGR})
    source_group("${SRCGR}" FILES ${f})
endforeach()

#==========================================
# Targets

add_executable(YatoTests ${all_sources})

target_link_libraries(YatoTests gtest)
target_link_libraries(YatoTests gtest_main)