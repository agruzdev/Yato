#
# YATO library
#
# Apache License, Version 2.0
# Copyright (c) 2016-2020 Alexey Gruzdev
#

project(YatoConfig)

#==========================================
# Core sources

file(GLOB config_sources
    "${CMAKE_CURRENT_SOURCE_DIR}/yato/config/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/yato/config/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/yato/config/private/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/yato/config/private/*.cpp"
)

list(APPEND Yato_TEST_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/tests")

list(APPEND Yato_TEST_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/tests/manual")
set(CONFIG_NON_EMPTY_TEST ON)

# Json
option(YATO_CONFIG_JSON "Enable json config backend" OFF)
if(YATO_CONFIG_JSON)
    include(${YATO_SOURCE_DIR}/cmake/dependency.json.cmake)
    include_directories(${JSON_INCLUDE_DIR})
    file(GLOB json_sources
        "${CMAKE_CURRENT_SOURCE_DIR}/yato/config/json/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/yato/config/json/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/yato/config/json/private/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/yato/config/json/private/*.cpp"
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
        "${CMAKE_CURRENT_SOURCE_DIR}/yato/config/xml/private/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/yato/config/xml/private/*.cpp"
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
    list(APPEND config_dependency_targets ${INI_LIBRARIES})
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
        "${CMAKE_CURRENT_SOURCE_DIR}/yato/config/yaml/private/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/yato/config/yaml/private/*.cpp"
    )
    list(APPEND config_sources ${yaml_sources})
    list(APPEND Yato_TEST_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/tests/yaml")
    set(CONFIG_NON_EMPTY_TEST ON)
endif()

include_directories(${YATO_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${config_sources})

if(CLANG)
    add_definitions("-D_SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING=1")
endif()

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

