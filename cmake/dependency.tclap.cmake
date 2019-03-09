#
# YATO library
#
# Apache License, Version 2.0
# Copyright (c) 2018 Alexey Gruzdev
#

# TCLAP dependency
# https://sourceforge.net/projects/tclap/

# Output variables:
# TCLAP_INCLUDE_DIR - includes
# TCLAP_LIBRARY_DIR - link directories
# TCLAP_LIBRARIES   - link targets

include(${YATO_SOURCE_DIR}/cmake/dependency.common.functions.cmake)

dependency_find_or_download(
    NAME TCLAP
    VERBOSE_NAME "TCLAP"
    URL "https://sourceforge.net/projects/tclap/files/tclap-1.2.2.tar.gz"
    HASH_MD5 "6f35665814dca292eceda007d7e13bcb"
    FILE_NAME "tclap-1.2.2.tar.gz"
    PREFIX "tclap-1.2.2"
)

set(TCLAP_INCLUDE_DIR ${TCLAP_FOUND_ROOT}/include CACHE INTERNAL "")
set(TCLAP_LIBRARY_DIR "" CACHE INTERNAL "")
set(TCLAP_LIBRARIES "" CACHE INTERNAL "")
