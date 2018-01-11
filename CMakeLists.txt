cmake_minimum_required (VERSION 3.1)

project(Yato)

#==========================================
# Output

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${BIN_OUTPUT_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${BIN_OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BIN_OUTPUT_DIR})

#==========================================
# Determine target

if(MSVC)
    set(TARGET_CONFIG_LISTS ${CMAKE_SOURCE_DIR}/cmake/CMakeLists-MSVC.cmake)
elseif(UNIX OR MINGW OR ANDROID)
    set(TARGET_CONFIG_LISTS ${CMAKE_SOURCE_DIR}/cmake/CMakeLists-GCC.cmake)
endif()

#==========================================
# Core sources

set(YATO_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/include")

file(GLOB_RECURSE yato_sources "${YATO_INCLUDE_DIRS}/*.h")


if(UNIX)
    option(YATO_WITH_ADDRESS_SANITIZER "Enable address sanitizing" OFF)
    if(YATO_WITH_ADDRESS_SANITIZER)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer -g -fsanitize=address")
    endif()
endif()


#==========================================
# Targets

#ToDo (a.gruzdev) Temporal workaround
if(UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-noexcept-type")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-format-security")
endif()


if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()
message(STATUS "CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}")

option(YATO_BUILD_ACTORS "Build actors module" OFF)
if(YATO_BUILD_ACTORS)
    option(YATO_ACTORS_WITH_IO "Enable networking component" OFF)

    add_subdirectory(./actors)
    list(APPEND YATO_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/actors")
endif()

option(YATO_BUILD_TESTS "Build unit tests" ON)
if(YATO_BUILD_TESTS)
    add_subdirectory(./tests)
endif()