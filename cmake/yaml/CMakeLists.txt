#
# YATO library
#
# Apache License, Version 2.0
# Copyright (c) 2018 Alexey Gruzdev
#

# Use the common set of flags
include(${TARGET_CONFIG_LISTS})
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W0")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w")
endif()

file(GLOB yaml_sources "${YAML_FOUND_ROOT}/src/*.h" "${YAML_FOUND_ROOT}/src/*.cpp" "${YAML_FOUND_ROOT}/include/yaml-cpp/*.h")

include_directories(yamlcpp PRIVATE "${YAML_FOUND_ROOT}/include")

add_definitions(-D_NOEXCEPT=noexcept)

add_library(yamlcpp STATIC ${yaml_sources})




