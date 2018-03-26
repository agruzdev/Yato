project(YatoConfig)

include(${TARGET_CONFIG_LISTS})

#==========================================
# Core sources

file(GLOB config_sources
    "${CMAKE_CURRENT_SOURCE_DIR}/yato/config/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/yato/config/*.cpp"
)

option(YATO_CONFIG_MANUAL "Enable manual config" OFF)
if(YATO_CONFIG_MANUAL)
    file(GLOB manual_sources
        "${CMAKE_CURRENT_SOURCE_DIR}/yato/config/manual/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/yato/config/manual/*.cpp"
    )
    list(APPEND config_sources ${manual_sources})
endif()

option(YATO_CONFIG_JSON "Enable json config backend" OFF)
if(YATO_CONFIG_JSON)
    include(${CMAKE_SOURCE_DIR}/cmake/dependency.json.cmake)
    include_directories(${DEPENDS_JSON_INCLUDE_DIR})
    file(GLOB json_sources
        "${CMAKE_CURRENT_SOURCE_DIR}/yato/config/json/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/yato/config/json/*.cpp"
    )
    list(APPEND config_sources ${json_sources})
endif()


include_directories(${YATO_INCLUDE_DIRS})
include_directories("${CMAKE_CURRENT_SOURCE_DIR}")

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${config_sources})

add_library(YatoConfig STATIC ${config_sources})

if(YATO_CONFIG_JSON)
    add_dependencies(YatoConfig Json)
endif()

#===========================================
# Tests

if(YATO_CONFIG_MANUAL)
    list(APPEND Yato_TEST_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/tests/manual")
endif()
if(YATO_CONFIG_JSON)
    list(APPEND Yato_TEST_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/tests/json")
endif()
set(Yato_TEST_DIRS ${Yato_TEST_DIRS} PARENT_SCOPE)

list(APPEND Yato_TEST_LIBS YatoConfig)
set(Yato_TEST_LIBS ${Yato_TEST_LIBS} PARENT_SCOPE)

