
project(YatoActors)

include(${TARGET_CONFIG_LISTS})

#==========================================
# Core sources

file(GLOB_RECURSE yato_actors_sources "${CMAKE_CURRENT_SOURCE_DIR}/**/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/**/*.cpp")

include_directories(${YATO_INCLUDE_DIRS})

if("${CMAKE_VERSION}" VERSION_GREATER 3.8.0)
    source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${yato_actors_sources})
endif()

add_library(YatoActors STATIC ${yato_actors_sources})


