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
    URL "https://github.com/nlohmann/json/releases/download/v3.1.2/include.zip"
    HASH_MD5 "0044fbef3d6b7087bb6c0498ac62acf8"
)

set(JSON_INCLUDE_DIR ${JSON_FOUND_ROOT}/include CACHE INTERNAL "")
set(JSON_LIBRARY_DIR "" CACHE INTERNAL "")
set(JSON_LIBRARIES "" CACHE INTERNAL "")

