
project(YatoActors)

include(${TARGET_CONFIG_LISTS})

#==========================================
# Core sources

file(GLOB_RECURSE yato_actors_sources "${CMAKE_CURRENT_SOURCE_DIR}/**/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/**/*.cpp")

include_directories(${YATO_INCLUDE_DIRS})

add_library(YatoActors STATIC ${yato_actors_sources})


