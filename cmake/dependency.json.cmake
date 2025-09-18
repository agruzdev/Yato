#
# YATO library
#
# Apache License, Version 2.0
# Copyright 2016-2020 Alexey Gruzdev
#

# JSON dependency
# https://github.com/nlohmann/json

# Output variables:
# JSON_INCLUDE_DIR - includes
# JSON_LIBRARY_DIR - link directories
# JSON_LIBRARIES   - link targets

include(${YATO_SOURCE_DIR}/cmake/dependency.common.functions.cmake)

dependency_find_or_download(
    NAME JSON
    VERBOSE_NAME "nlohmann::json"
    URL "https://github.com/nlohmann/json/releases/download/v3.12.0/include.zip"
    HASH_MD5 "b55682fb4e47835e7bbd3da7370c2b83"
)

set(JSON_INCLUDE_DIR ${JSON_FOUND_ROOT}/include CACHE INTERNAL "")
set(JSON_LIBRARY_DIR "" CACHE INTERNAL "")
set(JSON_LIBRARIES "" CACHE INTERNAL "")

