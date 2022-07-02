#
# YATO library
#
# Apache License, Version 2.0
# Copyright 2016-2020 Alexey Gruzdev
#

# INI parser dependency
# https://github.com/brofield/simpleini

# Output variables:
# INI_INCLUDE_DIR - includes
# INI_LIBRARY_DIR - link directories
# INI_LIBRARIES   - link targets

include(${YATO_SOURCE_DIR}/cmake/dependency.common.functions.cmake)

dependency_find_or_download(
    NAME INI
    VERBOSE_NAME "Inih"
    URL "https://github.com/benhoyt/inih/archive/refs/tags/r55.zip"
    HASH_MD5 "9dac903c128edbd0cb0c29e8818e08d0"
    FILE_NAME "inih-r55.zip"
    PREFIX "inih-r55"
)

if(NOT TARGET Inih)
    add_subdirectory(${CMAKE_SOURCE_DIR}/cmake/ini ${CMAKE_BINARY_DIR}/dependency/ini)
    set_property(TARGET Inih PROPERTY FOLDER "Dependencies")
endif()

set(INI_INCLUDE_DIR ${INI_FOUND_ROOT} CACHE INTERNAL "")
set(INI_LIBRARY_DIR "" CACHE INTERNAL "")
set(INI_LIBRARIES Inih CACHE INTERNAL "")
