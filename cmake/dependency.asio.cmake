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
    URL "https://github.com/chriskohlhoff/asio/archive/asio-1-12-0.zip"
    HASH_MD5 "e7b27761818ba597886620c87f48a768"
    PREFIX "asio-asio-1-12-0"
)

set(ASIO_INCLUDE_DIR ${ASIO_FOUND_ROOT}/asio/include CACHE INTERNAL "")
set(ASIO_LIBRARY_DIR "" CACHE INTERNAL "")
set(ASIO_LIBRARIES "" CACHE INTERNAL "")

