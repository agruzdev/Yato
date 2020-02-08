#
# YATO library
#
# Apache License, Version 2.0
# Copyright (c) 2018 Alexey Gruzdev
#

# YAML dependency
# https://github.com/jbeder/yaml-cpp

# Output variables:
# YAML_INCLUDE_DIR - includes
# YAML_LIBRARY_DIR - link directories
# YAML_LIBRARIES   - link targets

include(${YATO_SOURCE_DIR}/cmake/dependency.common.functions.cmake)

dependency_find_or_download(
    NAME YAML
    VERBOSE_NAME "yaml-cpp"
    URL "https://github.com/jbeder/yaml-cpp/archive/yaml-cpp-0.6.3.zip"
    HASH_MD5 "e8a182537af663cc45228f7064b2021c"
    PREFIX "yaml-cpp-yaml-cpp-0.6.3"
)

if(NOT TARGET yamlcpp)
    #file(GLOB yaml_sources "${YAML_FOUND_ROOT}/src/*.h" "${YAML_FOUND_ROOT}/src/*.cpp" "${YAML_FOUND_ROOT}/include/yaml-cpp/*.h")

    # Use the common set of flags
    #include(${TARGET_CONFIG_LISTS})

    #add_library(yamlcpp STATIC ${yaml_sources})
    #target_include_directories(yamlcpp PRIVATE "${YAML_FOUND_ROOT}/include")

    add_subdirectory(${CMAKE_SOURCE_DIR}/cmake/yaml ${CMAKE_BINARY_DIR}/dependency/yaml)
    set_property(TARGET yamlcpp PROPERTY FOLDER "Dependencies")
endif()

set(YAML_INCLUDE_DIR "${YAML_FOUND_ROOT}/include" CACHE INTERNAL "")
set(YAML_LIBRARY_DIR "" CACHE INTERNAL "")
set(YAML_LIBRARIES yamlcpp CACHE INTERNAL "")

