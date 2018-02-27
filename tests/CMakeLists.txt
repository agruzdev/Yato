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
list(APPEND all_sources ${yato_sources})

foreach(test_dir ${Yato_TEST_DIRS})
    message(STATUS "Added tests from: ${test_dir}")
    file(GLOB test_files "${test_dir}/*.cpp" "${test_dir}/*.h")
    list(APPEND all_sources ${test_files})
endforeach()

# source_group(TREE "${CMAKE_SOURCE_DIR}" FILES ${all_sources})

foreach(f ${all_sources})
    # Get the path of the file relative to ${CMAKE_SOURCE_DIR},
    # then alter it (not compulsory)
    file(RELATIVE_PATH SRCGR ${CMAKE_SOURCE_DIR} ${f})
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

#if(YATO_BUILD_ACTORS)
#    target_link_libraries(YatoTests YatoActors)
#endif()
foreach(test_lib ${Yato_TEST_LIBS})
    target_link_libraries(YatoTests ${test_lib})
endforeach()

_gtest_add_definitions(YatoTests)
