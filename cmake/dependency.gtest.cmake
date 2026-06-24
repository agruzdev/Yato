#
# YATO library
#
# Apache License, Version 2.0
# Copyright 2016-2020 Alexey Gruzdev
#
# Google test dependency
# https://github.com/google/googletest
#
# Targets:
# gtest and gtest_main

if (TARGET gtest)
    return()
endif()

macro(_gtest_fix_definitions _TARGET_NAME_)
    if(TARGET ${_TARGET_NAME_})
        target_compile_definitions(${_TARGET_NAME_} PRIVATE GTEST_LANG_CXX11=1 GTEST_HAS_TR1_TUPLE=0)
    endif()
endmacro(_gtest_fix_definitions)

find_package(GTest CONFIG)
if (TARGET GTest::gtest)
    add_library(gtest ALIAS GTest::gtest)
    add_library(gtest_main ALIAS GTest::gtest_main)
    return()
endif()

include(${YATO_SOURCE_DIR}/cmake/dependency.common.functions.cmake)

dependency_find_or_download(
    NAME GTEST
    VERBOSE_NAME "GoogleTest"
    URL "https://github.com/google/googletest/archive/refs/tags/release-1.12.1.zip"
    HASH_MD5 "2648d4138129812611cf6b6b4b497a3b"
    PREFIX "googletest-release-1.12.1"
)

if(NOT TARGET gtest)
    set(BUILD_GTEST ON  CACHE BOOL "gtest setup" FORCE)
    set(BUILD_GMOCK OFF CACHE BOOL "gtest setup" FORCE)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "gtest setup" FORCE)
    set(INSTALL_GTEST OFF CACHE BOOL "gtest setup" FORCE)
    set(gtest_disable_pthreads ON CACHE BOOL "gtest setup" FORCE)
    set(gtest_force_shared_crt ON CACHE BOOL "gtest setup" FORCE)

    add_subdirectory(${GTEST_FOUND_ROOT} ${CMAKE_BINARY_DIR}/dependency/gtest)

    #_gtest_fix_definitions(gtest)
    #_gtest_fix_definitions(gtest_main)

    if(CLANG_MSVC)
        target_compile_options(gtest      PRIVATE "-w")
        target_compile_options(gtest_main PRIVATE "-w")
    endif()

    set_property(TARGET gtest      PROPERTY FOLDER "Dependencies")
    set_property(TARGET gtest_main PROPERTY FOLDER "Dependencies")
endif()
