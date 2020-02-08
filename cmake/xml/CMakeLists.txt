#
# YATO library
#
# Apache License, Version 2.0
# Copyright 2016-2020 Alexey Gruzdev
#

list(APPEND tinyxml2_sources "${XML_FOUND_ROOT}/tinyxml2.h")
list(APPEND tinyxml2_sources "${XML_FOUND_ROOT}/tinyxml2.cpp")

# Use the common set of flags
include(${TARGET_CONFIG_LISTS})
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W0")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w")
endif()

add_library(tinyxml2 STATIC ${tinyxml2_sources})


