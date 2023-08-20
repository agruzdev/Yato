#
# YATO library
#
# Apache License, Version 2.0
# Copyright 2016-2020 Alexey Gruzdev
#

# MSVC

# ===============================================
# Compiler flags

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:${YATO_CXX_STANDARD}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D_SCL_SECURE_NO_WARNINGS")

option(YATO_USE_SHARED_RUNTIME "Link dynamic runtime" ON)
if(NOT YATO_USE_SHARED_RUNTIME)
    set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
endif()

