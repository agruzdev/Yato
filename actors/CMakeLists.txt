
project(YatoActors)

include(${TARGET_CONFIG_LISTS})

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
    find_package(Boost REQUIRED COMPONENTS system date_time regex)

    file(GLOB yato_actors_io_sources
        "${CMAKE_CURRENT_SOURCE_DIR}/yato/actors/io/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/yato/actors/io/private/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/yato/actors/io/private/*.cpp")

    list(APPEND yato_actors_sources ${yato_actors_io_sources})

    add_definitions(-DYATO_ACTORS_WITH_IO)

    include_directories(${Boost_INCLUDE_DIRS})
    link_directories(${Boost_LIBRARY_DIRS})

    if(WIN32 AND CLANG)
        add_definitions("-DBOOST_USE_WINDOWS_H")
    endif()
endif()

#=========================================


if(NOT "${CMAKE_VERSION}" VERSION_LESS 3.8.0)
    source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${yato_actors_sources})
endif()

add_library(YatoActors STATIC ${yato_actors_sources})

if(YATO_ACTORS_WITH_IO)
    target_link_libraries(YatoActors PRIVATE debug ${Boost_SYSTEM_LIBRARY_DEBUG} ${Boost_DATE_TIME_LIBRARY_DEBUG} ${Boost_REGEX_LIBRARY_DEBUG})
    target_link_libraries(YatoActors PRIVATE optimized ${Boost_SYSTEM_LIBRARY_RELEASE} ${Boost_DATE_TIME_LIBRARY_RELEASE} ${Boost_REGEX_LIBRARY_RELEASE})

    if(WIN32 AND NOT MSVC)
        target_link_libraries(YatoActors PRIVATE wsock32 ws2_32)
    endif()
endif()
