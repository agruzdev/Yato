#
# YATO library
#
# Apache License, Version 2.0
# Copyright (c) 2018 Alexey Gruzdev
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
    VERBOSE_NAME "SimpleIni"
    URL "https://github.com/brofield/simpleini/archive/4.17.zip"
    HASH_MD5 "b0c5f48400df3661e735d2c2924bff81"
    FILE_NAME "simpleini_4_17.zip"
    PREFIX "simpleini-4.17"
)

if(NOT TARGET SimpleIni)
    set(simpleini_sources "${INI_FOUND_ROOT}/SimpleIni.h"
                          "${INI_FOUND_ROOT}/ConvertUTF.h"
                          "${INI_FOUND_ROOT}/ConvertUTF.c"
    )

    # Use the common set of flags
    #include(${TARGET_CONFIG_LISTS})

    add_library(SimpleIni STATIC ${simpleini_sources})
    set_property(TARGET SimpleIni PROPERTY FOLDER "Dependencies")
endif()

set(INI_INCLUDE_DIR ${INI_FOUND_ROOT} CACHE INTERNAL "")
set(INI_LIBRARY_DIR "" CACHE INTERNAL "")
set(INI_LIBRARIES "" CACHE INTERNAL "")
