#
# YATO library
#
# Apache License, Version 2.0
# Copyright 2016-2020 Alexey Gruzdev
#

# GCC

# ===============================================
# Compiler flags

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=${YATO_CXX_STANDARD}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wpedantic")
if(CLANG)
    if(CLANG_MINGW OR ANDROID)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -femulated-tls")
    endif()
    if(CLANG_MINGW)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
    endif()
endif()

#ToDo (a.gruzdev) Temporal workaround
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-format-security")
if(NOT ANDROID)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-noexcept-type")
endif()


if(NOT ANDROID)
    option(YATO_WITH_ADDRESS_SANITIZER "Enable address sanitizing" OFF)
    if(YATO_WITH_ADDRESS_SANITIZER)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer -g -fsanitize=address")
    endif()
endif()

