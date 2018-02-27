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


include_directories(${YATO_INCLUDE_DIRS})
include_directories("${CMAKE_CURRENT_SOURCE_DIR}")

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${config_sources})

add_library(YatoConfig STATIC ${config_sources})

#===========================================
# Tests

if(YATO_CONFIG_MANUAL)
    list(APPEND YatoConfig_TEST_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/tests/manual")
    list(APPEND Yato_TEST_DIRS ${YatoConfig_TEST_DIRS})
endif()
set(Yato_TEST_DIRS ${Yato_TEST_DIRS} PARENT_SCOPE)

list(APPEND Yato_TEST_LIBS YatoConfig)
set(Yato_TEST_LIBS ${Yato_TEST_LIBS} PARENT_SCOPE)

