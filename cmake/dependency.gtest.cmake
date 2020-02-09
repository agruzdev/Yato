#
# YATO library
#
# Apache License, Version 2.0
# Copyright 2016-2020 Alexey Gruzdev
#
# Google test dependency
# https://github.com/google/googletest
#
# Output variables:
# GTEST_INCLUDE_DIR - includes
# GTEST_LIBRARY_DIR - link directories
# GTEST_LIBRARIES   - link targets
# GMOCK_INCLUDE_DIR - includes


include(${YATO_SOURCE_DIR}/cmake/dependency.common.functions.cmake)

macro(_gtest_fix_definitions _TARGET_NAME_)
    if(TARGET ${_TARGET_NAME_})
        target_compile_definitions(${_TARGET_NAME_} PRIVATE GTEST_LANG_CXX11=1 GTEST_HAS_TR1_TUPLE=0)
    endif()
endmacro(_gtest_fix_definitions)

dependency_find_or_download(
    NAME GTEST
    VERBOSE_NAME "GoogleTest"
    URL "https://github.com/google/googletest/archive/ba96d0b1161f540656efdaed035b3c062b60e006.zip"
    HASH_MD5 "bf88bc2ea057f6375fa71b68f907bcf7"
    PREFIX "googletest-ba96d0b1161f540656efdaed035b3c062b60e006"
)

if(NOT TARGET gtest)
    set(BUILD_GTEST ON  CACHE BOOL "gtest setup" FORCE)
    set(BUILD_GMOCK OFF CACHE BOOL "gtest setup" FORCE)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "gtest setup" FORCE)
    set(gtest_disable_pthreads ON CACHE BOOL "gtest setup" FORCE)
    set(gtest_force_shared_crt ON CACHE BOOL "gtest setup" FORCE)

    add_subdirectory(${GTEST_FOUND_ROOT}/googletest ${CMAKE_BINARY_DIR}/dependency/gtest)

    _gtest_fix_definitions(gtest)
    _gtest_fix_definitions(gtest_main)

    if(CLANG_MSVC)
        target_compile_options(gtest      PRIVATE "-w")
        target_compile_options(gtest_main PRIVATE "-w")
    endif()

    set_property(TARGET gtest      PROPERTY FOLDER "Dependencies")
    set_property(TARGET gtest_main PROPERTY FOLDER "Dependencies")
endif()

set(GTEST_INCLUDE_DIR ${GTEST_FOUND_ROOT}/googletest/include CACHE INTERNAL "")
set(GMOCK_INCLUDE_DIR ${GTEST_FOUND_ROOT}/googlemock/include CACHE INTERNAL "")
set(GTEST_LIBRARY_DIR "" CACHE INTERNAL "")
list(APPEND GTEST_LIBRARIES gtest)
list(APPEND GTEST_LIBRARIES gtest_main)
set(GTEST_LIBRARIES ${GTEST_LIBRARIES} CACHE INTERNAL "")


