project(YatoTests)

#==========================================
# Dependencies

set(GTEST_ROOT "$ENV{GTEST_ROOT}" CACHE PATH "Path to google-test dir with cmake lists")

add_subdirectory(${GTEST_ROOT} ${CMAKE_BINARY_DIR}/gtest)
set(GTEST_INCLUDE_DIR "${GTEST_ROOT}/googletest/include" CACHE PATH "Path to includes")
message(STATUS "gtest include = ${GTEST_INCLUDE_DIR}")


include(${TARGET_CONFIG_LISTS})

macro(_gtest_add_definitions _TARGET_NAME_)
    if(TARGET ${_TARGET_NAME_})
        target_compile_definitions(${_TARGET_NAME_} PRIVATE GTEST_LANG_CXX11=1 GTEST_HAS_TR1_TUPLE=0)
    endif()
endmacro(_gtest_add_definitions)

_gtest_add_definitions(gmock)
_gtest_add_definitions(gmock_main)
_gtest_add_definitions(gtest)
_gtest_add_definitions(gtest_main)

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

source_group(TREE "${CMAKE_SOURCE_DIR}" FILES ${all_sources})

#==========================================
# Targets

add_definitions(-DYATO_ENABLE_TESTED_ASSERTIONS)

link_libraries(gtest)
link_libraries(gtest_main)
foreach(test_lib ${Yato_TEST_LIBS})
    link_libraries(${test_lib})
endforeach()

add_executable(YatoTests ${all_sources})

_gtest_add_definitions(YatoTests)
