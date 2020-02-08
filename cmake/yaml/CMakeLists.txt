#
# YATO library
#
# Apache License, Version 2.0
# Copyright 2016-2020 Alexey Gruzdev
#


include(${TARGET_CONFIG_LISTS})
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W0")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w")
endif()

file(GLOB yaml_sources "${YAML_FOUND_ROOT}/src/*.h" "${YAML_FOUND_ROOT}/src/*.cpp" "${YAML_FOUND_ROOT}/include/yaml-cpp/*.h")

include_directories(yamlcpp PRIVATE "${YAML_FOUND_ROOT}/include")

add_library(yamlcpp STATIC ${yaml_sources})




