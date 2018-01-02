
project(YatoActors)

include(${TARGET_CONFIG_LISTS})

#==========================================
# Core sources

file(GLOB yato_actors_sources 
    "${CMAKE_CURRENT_SOURCE_DIR}/yato/actors/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/yato/actors/private/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/yato/actors/private/*.cpp")

include_directories(${YATO_INCLUDE_DIRS})

#==========================================
# IO

if(YATO_ACTORS_WITH_IO)
    find_package(Boost REQUIRED COMPONENTS system)

    file(GLOB yato_actors_io_sources
        "${CMAKE_CURRENT_SOURCE_DIR}/yato/actors/io/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/yato/actors/io/private/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/yato/actors/io/private/*.cpp")

    list(APPEND yato_actors_sources ${yato_actors_io_sources})

    add_definitions(-DYATO_ACTORS_WITH_IO)

    include_directories(${Boost_INCLUDE_DIR})
    link_directories(${Boost_LIBRARY_DIR})
endif()

#=========================================


if(NOT "${CMAKE_VERSION}" VERSION_LESS 3.8.0)
    source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${yato_actors_sources})
endif()

add_library(YatoActors STATIC ${yato_actors_sources})

if(NOT Boost_SYSTEM_LIBRARY_DEBUG OR NOT Boost_SYSTEM_LIBRARY_RELEASE)
    message(FATAL_ERROR "Boost was not found!")
endif()

target_link_libraries(YatoActors PRIVATE debug ${Boost_SYSTEM_LIBRARY_DEBUG})
target_link_libraries(YatoActors PRIVATE optimized ${Boost_SYSTEM_LIBRARY_RELEASE})
