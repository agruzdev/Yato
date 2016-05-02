cmake_minimum_required (VERSION 3.1)

project(Yato)

#==========================================
# Output

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${BIN_OUTPUT_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${BIN_OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BIN_OUTPUT_DIR})

#==========================================
# Determine target

if(MSVC)
    set(TARGET_CONFIG_LISTS ${CMAKE_SOURCE_DIR}/cmake/CMakeLists-MSVC.cmake)
elseif(UNIX OR MINGW OR ANDROID)
    set(TARGET_CONFIG_LISTS ${CMAKE_SOURCE_DIR}/cmake/CMakeLists-GCC.cmake)
endif()


#==========================================
# Targets

option(YATO_BUILD_TESTS "Build unit tests" ON)
if(YATO_BUILD_TESTS)
    add_subdirectory(./tests)
endif()