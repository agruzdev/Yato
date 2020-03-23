#
# YATO library
#
# Apache License, Version 2.0
# Copyright (c) 2016-2020 Alexey Gruzdev
#

project(YatoActors)

include(${TARGET_CONFIG_LISTS})

#==========================================
# Dependency

if(NOT YATO_BUILD_CONFIG)
    message(FATAL_ERROR "YatoConf is required! Please enable YATO_BUILD_CONFIG")
endif()
include_directories(${YATO_SOURCE_DIR}/modules/config)

#==========================================
# Core sources

file(GLOB yato_actors_sources 
    "${CMAKE_CURRENT_SOURCE_DIR}/yato/actors/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/yato/actors/private/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/yato/actors/private/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/yato/actors/private/actors/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/yato/actors/private/actors/*.cpp")

include_directories(${YATO_INCLUDE_DIRS})

#==========================================
# IO

if(YATO_ACTORS_WITH_IO)
    include(${YATO_SOURCE_DIR}/cmake/dependency.asio.cmake)

    file(GLOB yato_actors_io_sources
        "${CMAKE_CURRENT_SOURCE_DIR}/yato/actors/io/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/yato/actors/io/private/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/yato/actors/io/private/*.cpp")
    list(APPEND yato_actors_sources ${yato_actors_io_sources})

    include_directories(${ASIO_INCLUDE_DIR})

    add_definitions(-DYATO_ACTORS_WITH_IO)
    add_definitions(-DASIO_STANDALONE)
    if(WIN32)
        add_definitions("-D_WIN32_WINNT=0x0501")
    endif()

    if(MINGW AND NOT CLANG)
       # Mingw 8.1 bug workaround
       set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-cast-function-type")
    endif()
endif()

#=========================================

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${yato_actors_sources})

add_library(YatoActors STATIC ${yato_actors_sources})
set_property(TARGET YatoActors PROPERTY FOLDER "Modules")

if(YATO_ACTORS_WITH_IO)
    if(WIN32)
        target_link_libraries(YatoActors PRIVATE wsock32)
        target_link_libraries(YatoActors PRIVATE ws2_32)
    elseif(UNIX AND NOT ANDROID)
        target_link_libraries(YatoActors PRIVATE pthread)
    endif()
endif()

#==========================================
# Tests

list(APPEND YatoActors_TEST_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/tests")
if(YATO_ACTORS_WITH_IO)
    list(APPEND YatoActors_TEST_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/tests/io")
endif()
list(APPEND Yato_TEST_DIRS ${YatoActors_TEST_DIRS})
set(Yato_TEST_DIRS ${Yato_TEST_DIRS} PARENT_SCOPE)

list(APPEND Yato_TEST_LIBS YatoActors)
set(Yato_TEST_LIBS ${Yato_TEST_LIBS} PARENT_SCOPE)

