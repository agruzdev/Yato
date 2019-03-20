#
# YATO library
#
# Apache License, Version 2.0
# Copyright (c) 2018 Alexey Gruzdev
#

cmake_minimum_required(VERSION 3.0)

if(CMAKE_TOOLCHAIN_FILE)
    # touch toolchain variable to suppress "unused variable" warning
endif()

if(NOT TOOLCHAIN_USED)

    if(NOT ANDROID_TOOLCHAIN)
        set(ANDROID_TOOLCHAIN $ENV{ANDROID_TOOLCHAIN})
    endif()
    if(NOT ANDROID_TOOLCHAIN)
        message(FATAL_ERROR "Failed to find android toolchian. Please set variable ANDROID_TOOLCHAIN to correct toolchain path")
    endif()
    file(TO_CMAKE_PATH ${ANDROID_TOOLCHAIN} ANDROID_TOOLCHAIN)
    set(ANDROID_TOOLCHAIN ${ANDROID_TOOLCHAIN} CACHE STRING "Android toolchain")
    message(STATUS "ANDROID_TOOLCHAIN=${ANDROID_TOOLCHAIN}")

    set(CMAKE_SYSTEM_NAME Linux)
    set_property(GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS TRUE)

    if(WIN32)
        set(OS_SUFFIX_COMPILER ".cmd")
        set(OS_SUFFIX_OTHER ".exe")
    else()
        set(OS_SUFFIX_COMPILER "")
        set(OS_SUFFIX_OTHER "")
    endif()

    if(NOT ANDROID_ARCH)
        set(ANDROID_ARCH $ENV{ANDROID_ARCH})
    endif()
    if(NOT ANDROID_ARCH)
        set(ANDROID_ARCH "aarch64")
    endif()

    set(ANDROID_ARCH ${ANDROID_ARCH} CACHE STRING "Android target arch")

    # Detect available API
    file(GLOB compilers ${ANDROID_TOOLCHAIN}/bin/${ANDROID_ARCH}-*-android[0-9][0-9]-clang${OS_SUFFIX_COMPILER})
    foreach(c ${compilers})
        string(REGEX REPLACE ".*-android([0-9]+)-.*" "\\1" num ${c})
        list(APPEND available_api ${num})
    endforeach()
    message(STATUS "Available API=${available_api}")
    list(LENGTH available_api api_count)

    if(${api_count} GREATER "0")
        if(NOT ANDROID_API)
            set(ANDROID_API $ENV{ANDROID_API})
        endif()
        list(FIND available_api "${ANDROID_API}" api_index)
        if("${api_index}" LESS "0")
            math(EXPR api_index "${api_count} - 1")
            list(GET available_api ${api_index} ANDROID_API)
        endif()

        set(ANDROID_API ${ANDROID_API} CACHE STRING "Android API level")

        # New style NDK
        file(GLOB toolchain_c_compiler   ${ANDROID_TOOLCHAIN}/bin/${ANDROID_ARCH}-*-android${ANDROID_API}-clang${OS_SUFFIX_COMPILER})
        file(GLOB toolchain_cxx_compiler ${ANDROID_TOOLCHAIN}/bin/${ANDROID_ARCH}-*-android${ANDROID_API}-clang++${OS_SUFFIX_COMPILER})
    endif()

    if(NOT (EXISTS "${toolchain_c_compiler}" AND EXISTS "${toolchain_cxx_compiler}"))
        # Old style NDK standalone toolchain
        file(GLOB toolchain_c_compiler   ${ANDROID_TOOLCHAIN}/bin/${ANDROID_ARCH}-*-android-clang${OS_SUFFIX_COMPILER})
        file(GLOB toolchain_cxx_compiler ${ANDROID_TOOLCHAIN}/bin/${ANDROID_ARCH}-*-android-clang++${OS_SUFFIX_COMPILER})
        unset(ANDROID_API CACHE)
    endif()

    file(GLOB toolchain_linker       ${ANDROID_TOOLCHAIN}/bin/${ANDROID_ARCH}-*-android-ld.gold${OS_SUFFIX_OTHER})
    file(GLOB toolchain_ar           ${ANDROID_TOOLCHAIN}/bin/${ANDROID_ARCH}-*-android-ar${OS_SUFFIX_OTHER})
    file(GLOB toolchain_nm           ${ANDROID_TOOLCHAIN}/bin/${ANDROID_ARCH}-*-android-nm${OS_SUFFIX_OTHER})
    file(GLOB toolchain_objcopy      ${ANDROID_TOOLCHAIN}/bin/${ANDROID_ARCH}-*-android-objcopy${OS_SUFFIX_OTHER})
    file(GLOB toolchain_objdump      ${ANDROID_TOOLCHAIN}/bin/${ANDROID_ARCH}-*-android-objdump${OS_SUFFIX_OTHER})
    file(GLOB toolchain_ranlib       ${ANDROID_TOOLCHAIN}/bin/${ANDROID_ARCH}-*-android-ranlib${OS_SUFFIX_OTHER})
    file(GLOB toolchain_strip        ${ANDROID_TOOLCHAIN}/bin/${ANDROID_ARCH}-*-android-strip${OS_SUFFIX_OTHER})

    function(check_valid_path path)
        list(LENGTH ${path} len)
        if(NOT ("${len}" EQUAL "1"))
            message(FATAL_ERROR "Invalid path ${path}=${${path}}")
        endif()
        if(NOT EXISTS "${${path}}")
            message(FATAL_ERROR "Path doesn't exist ${path}=${${path}}")
        endif()
    endfunction()

    check_valid_path(toolchain_c_compiler)
    check_valid_path(toolchain_cxx_compiler)
    check_valid_path(toolchain_linker)
    check_valid_path(toolchain_ar)
    check_valid_path(toolchain_nm)
    check_valid_path(toolchain_objcopy)
    check_valid_path(toolchain_objdump)
    check_valid_path(toolchain_ranlib)
    check_valid_path(toolchain_strip)

    set(CMAKE_C_COMPILER    ${toolchain_c_compiler}   CACHE STRING "Path to program")
    set(CMAKE_CXX_COMPILER  ${toolchain_cxx_compiler} CACHE STRING "Path to program")
    set(CMAKE_LINKER        ${toolchain_linker}       CACHE STRING "Path to program")
    set(CMAKE_NM            ${toolchain_nm}           CACHE STRING "Path to program")
    set(CMAKE_AR            ${toolchain_ar}           CACHE STRING "Path to program")
    set(CMAKE_OBJCOPY       ${toolchain_objcopy}      CACHE STRING "Path to program")
    set(CMAKE_OBJDUMP       ${toolchain_objdump}      CACHE STRING "Path to program")
    set(CMAKE_RANLIB        ${toolchain_ranlib}       CACHE STRING "Path to program")
    set(CMAKE_STRIP         ${toolchain_strip}        CACHE STRING "Path to program")

    
    set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -fpic -std=c11 -Wno-unused-command-line-argument "   CACHE STRING "C flags")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpic -std=c++17 " CACHE STRING "C++ flags")
    set(CMAKE_SHARED_LINKER_FLAGS  "${CMAKE_SHARED_LINKER_FLAGS} -fPIE " CACHE STRING "shared linker flags")
    set(CMAKE_EXE_LINKER_FLAGS     "${CMAKE_EXE_LINKER_FLAGS} -fPIE -pie " CACHE STRING "exe linker flags")

    set(CMAKE_C_STANDARD_LIBRARIES   "${CMAKE_C_STANDARD_LIBRARIES}  -lgcc "    CACHE STRING "Standard C Libraries")
    set(CMAKE_CXX_STANDARD_LIBRARIES "${MAKE_CXX_STANDARD_LIBRARIES} -lstdc++ " CACHE STRING "Standard C++ Libraries")

    set(TOOLCHAIN_USED TRUE CACHE INTERNAL "")
endif()

set(CLANG ON)
set(ANDROID ON)


