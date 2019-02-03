#
# YATO library
#
# The MIT License (MIT)
# Copyright (c) 2018 Alexey Gruzdev
#

# Asio dependency
# https://github.com/chriskohlhoff/asio/

# Output variables:
# ASIO_INCLUDE_DIR - includes
# ASIO_LIBRARY_DIR - link directories
# ASIO_LIBRARIES   - link targets

include(${YATO_SOURCE_DIR}/cmake/dependency.common.functions.cmake)

dependency_find_or_download(
    NAME ASIO
    VERBOSE_NAME "Asio"
    URL "https://github.com/chriskohlhoff/asio/archive/asio-1-12-2.zip"
    HASH_MD5 "001b897add76fc39480841be8813bf72"
    PREFIX "asio-asio-1-12-2"
)

set(ASIO_INCLUDE_DIR ${ASIO_FOUND_ROOT}/asio/include CACHE INTERNAL "")
set(ASIO_LIBRARY_DIR "" CACHE INTERNAL "")
set(ASIO_LIBRARIES "" CACHE INTERNAL "")

