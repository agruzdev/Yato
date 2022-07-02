#
# YATO library
#
# Apache License, Version 2.0
# Copyright 2016-2020 Alexey Gruzdev
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
    URL "https://sourceforge.net/projects/tclap/files/tclap-1.2.5.tar.gz"
    HASH_MD5 "346a92acf9b364dfbff0a6df03c8a59e"
    FILE_NAME "tclap-1.2.5.tar.gz"
    PREFIX "tclap-1.2.5"
)

set(TCLAP_INCLUDE_DIR ${TCLAP_FOUND_ROOT}/include CACHE INTERNAL "")
set(TCLAP_LIBRARY_DIR "" CACHE INTERNAL "")
set(TCLAP_LIBRARIES "" CACHE INTERNAL "")
