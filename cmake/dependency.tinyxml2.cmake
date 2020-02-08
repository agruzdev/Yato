#
# YATO library
#
# Apache License, Version 2.0
# Copyright 2016-2020 Alexey Gruzdev
#

# XML dependency
# https://github.com/leethomason/tinyxml2

# Output variables:
# XML_INCLUDE_DIR - includes
# XML_LIBRARY_DIR - link directories
# XML_LIBRARIES   - link targets

include(${YATO_SOURCE_DIR}/cmake/dependency.common.functions.cmake)

dependency_find_or_download(
    NAME XML
    VERBOSE_NAME "tinyxml2"
    URL "https://github.com/leethomason/tinyxml2/archive/6.2.0.zip"
    HASH_MD5 "dbf022eca0f698ea3a0fdea4c6bd5c5c"
    PREFIX "tinyxml2-6.2.0"
)

if(NOT TARGET tinyxml2)
    add_subdirectory(${CMAKE_SOURCE_DIR}/cmake/xml ${CMAKE_BINARY_DIR}/dependency/xml)
    set_property(TARGET tinyxml2 PROPERTY FOLDER "Dependencies")
endif()

set(XML_INCLUDE_DIR ${XML_FOUND_ROOT} CACHE INTERNAL "")
set(XML_LIBRARY_DIR "" CACHE INTERNAL "")
set(XML_LIBRARIES tinyxml2 CACHE INTERNAL "")

