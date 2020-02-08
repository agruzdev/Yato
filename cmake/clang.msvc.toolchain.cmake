#
# YATO library
#
# Apache License, Version 2.0
# Copyright (c) 2018 Alexey Gruzdev
#

cmake_minimum_required(VERSION 3.0)

if(DEFINED TOOLCHAIN_USED)
    return()
endif()
set(TOOLCHAIN_USED TRUE)

if(CMAKE_TOOLCHAIN_FILE)
    # touch toolchain variable to suppress "unused variable" warning
endif()

set(CMAKE_C_COMPILER   clang   CACHE STRING "C compiler")
set(CMAKE_CXX_COMPILER clang++ CACHE STRING "C++ compiler")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 " CACHE STRING "C++ flags")

set(CLANG ON)
set(CLANG_MSVC ON)
