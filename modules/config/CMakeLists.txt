#
# YATO library
#
# Apache License, Version 2.0
# Copyright (c) 2016-2019 Alexey Gruzdev
#

project(YatoConfig)

include(${TARGET_CONFIG_LISTS})

#==========================================
# Core sources

file(GLOB config_sources
    "${CMAKE_CURRENT_SOURCE_DIR}/yato/config/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/yato/config/*.cpp"
)

list(APPEND Yato_TEST_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/tests")

# Manual
option(YATO_CONFIG_MANUAL "Enable manual config" ON)
if(YATO_CONFIG_MANUAL)
    file(GLOB manual_sources
        "${CMAKE_CURRENT_SOURCE_DIR}/yato/config/manual/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/yato/config/manual/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/yato/config/manual/private/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/yato/config/manual/private/*.cpp"
    )
    list(APPEND config_sources ${manual_sources})
    list(APPEND Yato_TEST_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/tests/manual")
    set(CONFIG_NON_EMPTY_TEST ON)
endif()

# Json
option(YATO_CONFIG_JSON "Enable json config backend" OFF)
if(YATO_CONFIG_JSON)
    include(${YATO_SOURCE_DIR}/cmake/dependency.json.cmake)
    include_directories(${JSON_INCLUDE_DIR})
    file(GLOB json_sources
        "${CMAKE_CURRENT_SOURCE_DIR}/yato/config/json/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/yato/config/json/*.cpp"
    )
    list(APPEND config_sources ${json_sources})
    list(APPEND Yato_TEST_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/tests/json")
    set(CONFIG_NON_EMPTY_TEST ON)
endif()

# Command line
option(YATO_CONFIG_CMD "Enable command line config background" OFF)
if(YATO_CONFIG_CMD)
    include(${YATO_SOURCE_DIR}/cmake/dependency.tclap.cmake)
    include_directories(${TCLAP_INCLUDE_DIR})
    file(GLOB cmd_sources
        "${CMAKE_CURRENT_SOURCE_DIR}/yato/config/cmd/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/yato/config/cmd/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/yato/config/cmd/private/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/yato/config/cmd/private/*.cpp"
    )
    list(APPEND config_sources ${cmd_sources})
    list(APPEND Yato_TEST_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/tests/cmd")
    set(CONFIG_NON_EMPTY_TEST ON)
endif()

# XML
option(YATO_CONFIG_XML "Enable XML config background" OFF)
if(YATO_CONFIG_XML)
    include(${YATO_SOURCE_DIR}/cmake/dependency.tinyxml2.cmake)
    include_directories(${XML_INCLUDE_DIR})
    link_directories(${XML_LIBRARY_DIR})
    list(APPEND config_dependency_targets ${XML_LIBRARIES})
    file(GLOB xml_sources
        "${CMAKE_CURRENT_SOURCE_DIR}/yato/config/xml/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/yato/config/xml/*.cpp"
    )
    list(APPEND config_sources ${xml_sources})
    list(APPEND Yato_TEST_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/tests/xml")
    set(CONFIG_NON_EMPTY_TEST ON)
endif()

# INI
option(YATO_CONFIG_INI "Enable INI config background" OFF)
if(YATO_CONFIG_INI)
    include(${YATO_SOURCE_DIR}/cmake/dependency.ini.cmake)
    include_directories(${INI_INCLUDE_DIR})
    file(GLOB ini_sources
        "${CMAKE_CURRENT_SOURCE_DIR}/yato/config/ini/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/yato/config/ini/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/yato/config/ini/private/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/yato/config/ini/private/*.cpp"
    )
    list(APPEND config_sources ${ini_sources})
    list(APPEND Yato_TEST_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/tests/ini")
    set(CONFIG_NON_EMPTY_TEST ON)
endif()

# YAML
option(YATO_CONFIG_YAML "Enable YAML config background" OFF)
if(YATO_CONFIG_YAML)
    include(${YATO_SOURCE_DIR}/cmake/dependency.yaml.cmake)
    include_directories(${YAML_INCLUDE_DIR})
    link_directories(${YAML_LIBRARY_DIR})
    list(APPEND config_dependency_targets ${YAML_LIBRARIES})
    file(GLOB yaml_sources
        "${CMAKE_CURRENT_SOURCE_DIR}/yato/config/yaml/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/yato/config/yaml/*.cpp"
    )
    list(APPEND config_sources ${yaml_sources})
    list(APPEND Yato_TEST_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/tests/yaml")
    set(CONFIG_NON_EMPTY_TEST ON)
endif()

include_directories(${YATO_INCLUDE_DIRS})
include_directories("${CMAKE_CURRENT_SOURCE_DIR}")

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${config_sources})

add_library(YatoConfig STATIC ${config_sources})
set_property(TARGET YatoConfig PROPERTY FOLDER "Modules")


#============================================
# Dependencies

foreach(target ${config_dependency_targets})
    message(STATUS "YatoConf: added dependency ${target}")
    target_link_libraries(YatoConfig ${target})
endforeach()

#===========================================
# Tests

if(CONFIG_NON_EMPTY_TEST)
    list(APPEND Yato_TEST_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/tests/include")
endif()
set(Yato_TEST_DIRS ${Yato_TEST_DIRS} PARENT_SCOPE)

list(APPEND Yato_TEST_LIBS YatoConfig)
set(Yato_TEST_LIBS ${Yato_TEST_LIBS} PARENT_SCOPE)

