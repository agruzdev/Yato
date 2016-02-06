cmake_minimum_required (VERSION 3.1)

project(YatoTests)

set(GTEST_ROOT "$ENV{GTEST_ROOT}" CACHE PATH "Path to google-test dir with cmake lists")

add_subdirectory(${GTEST_ROOT} ${CMAKE_BINARY_DIR}/gtest)
set(GTEST_INCLUDE_DIR "${GTEST_ROOT}/googletest/include" CACHE PATH "Path to includes")
message(STATUS "gtest include = ${GTEST_INCLUDE_DIR}")

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
    if(MSVC_VERSION EQUAL 1800)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D_SCL_SECURE_NO_WARNINGS")
    endif()
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNDEBUG")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
endif()

include_directories(${GTEST_INCLUDE_DIR})
include_directories("${CMAKE_SOURCE_DIR}/include")

file(GLOB_RECURSE all_tests "${CMAKE_SOURCE_DIR}/tests/*.cpp" "${CMAKE_SOURCE_DIR}/tests/*.h")
file(GLOB_RECURSE yato_sources "${CMAKE_SOURCE_DIR}/include/*.h")
list(APPEND all_sources ${all_tests})
list(APPEND all_sources ${yato_sources})

add_executable(YatoTests ${all_sources})

target_link_libraries(YatoTests gtest)
target_link_libraries(YatoTests gtest_main)

if(MSVC)
    file(GLOB all_dlls_dbg "${GTEST_BINARY_DIR}/Debug/*.dll")
    install(FILES ${all_dlls_dbg}
		DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/Debug/"
		CONFIGURATIONS Debug 
	)
 
    file(GLOB all_dlls_rel "${GTEST_BINARY_DIR}/Release/*.dll")
	install(FILES ${all_dlls_rel}
		DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/Release/"
		CONFIGURATIONS Release
	)
endif()