#
# YATO library
#
# Apache License, Version 2.0
# Copyright 2016-2020 Alexey Gruzdev
#

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W0")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w")
endif()

add_definitions("-DINI_USE_STACK=0")
add_definitions("-DINI_ALLOW_MULTILINE=0")
add_definitions("-DINI_MAX_LINE=4096")
add_definitions("-DINI_ALLOW_REALLOC=1")

set(inih_sources "${INI_FOUND_ROOT}/ini.h"
                      "${INI_FOUND_ROOT}/ini.c"
                      "${INI_FOUND_ROOT}/cpp/INIReader.h"
                      "${INI_FOUND_ROOT}/cpp/INIReader.cpp"
)

add_library(Inih STATIC ${inih_sources})
