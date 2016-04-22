cmake_minimum_required (VERSION 3.1)

# ==============================================================
# Usage
#   -D_TARGET=<target name>
# Supported targets:
#   vc12x32  - MSVC_2013 x32
#   vc14x32  - MSVC_2015 x32
#   vc12x64  - MSVC_2013 x64
#   vc14x64  - MSVC_2015 x64
#   mingw    - MinGW
#   gcc      - unix GCC 
#   all      - enable all listed (filtered by operating system)

# ==============================================================
# Global variables
#

if(WIN32)
    list(APPEND SUPPORTED_TARGETS mingw)
    if(IS_DIRECTORY $ENV{VS120COMNTOOLS})
        list(APPEND SUPPORTED_TARGETS vc12x32)
        list(APPEND SUPPORTED_TARGETS vc12x64)
    endif()
    if(IS_DIRECTORY $ENV{VS140COMNTOOLS})
        list(APPEND SUPPORTED_TARGETS vc14x32)
        list(APPEND SUPPORTED_TARGETS vc14x64)
    endif()
endif()

if(UNIX)
    list(APPEND SUPPORTED_TARGETS gcc)
endif()

set(GENERATOR_mingw "MinGW Makefiles")
set(GENERATOR_vc12x32 "Visual Studio 12 2013")
set(GENERATOR_vc12x64 "Visual Studio 12 2013 Win64")
set(GENERATOR_vc14x32 "Visual Studio 14 2015")
set(GENERATOR_vc14x64 "Visual Studio 14 2015 Win64")
set(GENERATOR_gcc "Unix Makefiles")

set(MAKE_COMMAND_mingw "mingw32-make")
set(MAKE_COMMAND_vc12x32 "$ENV{VS120COMNTOOLS}/../IDE/devenv.com")
set(MAKE_COMMAND_vc12x64 "$ENV{VS120COMNTOOLS}/../IDE/devenv.com")
set(MAKE_COMMAND_vc14x32 "$ENV{VS140COMNTOOLS}/../IDE/devenv.com")
set(MAKE_COMMAND_vc14x64 "$ENV{VS140COMNTOOLS}/../IDE/devenv.com")
set(MAKE_COMMAND_gcc "make")

list(APPEND MAKE_ARGUMENTS_mingw "-j" "2")
list(APPEND MAKE_ARGUMENTS_vc12x32 "/build" "Release")
list(APPEND MAKE_ARGUMENTS_vc12x64 "/build" "Release")
list(APPEND MAKE_ARGUMENTS_vc14x32 "/build" "Release")
list(APPEND MAKE_ARGUMENTS_vc14x64 "/build" "Release")
list(APPEND MAKE_ARGUMENTS_gcc "-j" "2")

# ==============================================================
# Input argumets
#

set(error FALSE)
if(NOT DEFINED _TARGET)
    set(error TRUE)
elseif(NOT _TARGET STREQUAL all)
    list(FIND SUPPORTED_TARGETS ${_TARGET} TARGET_IDX)
    if(TARGET_IDX EQUAL -1)
        set(error TRUE)
    endif()
endif()

if(error)
    message(STATUS "Error! No build target is specified!")
    message(STATUS "Supported targets: ${SUPPORTED_TARGETS}")
    message(STATUS "Aborting")
    return()
endif()
unset(error)

if(NOT DEFINED _SOURCE_DIR)
    set(_SOURCE_DIR ${CMAKE_SOURCE_DIR})
endif()

if(NOT DEFINED _BUILD_DIR)
    set(_BUILD_DIR ${_SOURCE_DIR}/cmake_workspace/build)
endif()
file(MAKE_DIRECTORY ${_BUILD_DIR})

if(NOT DEFINED _BIN_DIR)
    set(_BIN_DIR ${_SOURCE_DIR}/cmake_workspace/bin)
endif()
file(MAKE_DIRECTORY ${_BIN_DIR})

if(_TARGET STREQUAL all)
    set(all_build_targers ${SUPPORTED_TARGETS})
else()
    set(all_build_targers ${_TARGET})
endif()

# ==============================================================
# Make all targets
#

macro(CHECK_RETURN_CODE _ret_code)
    if(${_ret_code} EQUAL 0)
        message(STATUS OK)
    else()
        message(STATUS "ERROR! Return code ${${ret}}")
    endif()
endmacro()

foreach(CURRENT_TARGET ${all_build_targers})
    message(STATUS "------------------------------------------------------")
    
    set(CURRENT_BUILD_DIR ${_BUILD_DIR}/${CURRENT_TARGET})
    set(CURRENT_BIN_DIR ${_BIN_DIR}/${CURRENT_TARGET})
    
    file(REMOVE_RECURSE ${CURRENT_BUILD_DIR} ${CURRENT_BIN_DIR})
    file(MAKE_DIRECTORY ${CURRENT_BUILD_DIR} ${CURRENT_BIN_DIR}) 
    
    # ==============================================================
    # Configure
    #
    message(STATUS "Configuring for: ${CURRENT_TARGET}") 

    execute_process(COMMAND cmake "-G${GENERATOR_${CURRENT_TARGET}}" -DBIN_OUTPUT_DIR=${CURRENT_BIN_DIR} ${_SOURCE_DIR}
        WORKING_DIRECTORY ${CURRENT_BUILD_DIR}
        OUTPUT_FILE ${CURRENT_BUILD_DIR}/config_log.stdout.txt
        ERROR_FILE ${CURRENT_BUILD_DIR}/config_log.stderr.txt
        RESULT_VARIABLE ret
    )
    CHECK_RETURN_CODE(ret)
    
    
    # ==============================================================
    # Build
    #
    if(ret EQUAL 0)
        message(STATUS "Building: ${CURRENT_TARGET}") 
        
        # Find solution files
        if(CURRENT_TARGET MATCHES vc*)
            set(msvc_project ${CURRENT_BUILD_DIR}/ALL_BUILD.vcxproj)
        endif() 
        
        execute_process(COMMAND ${MAKE_COMMAND_${CURRENT_TARGET}} ${msvc_project} ${MAKE_ARGUMENTS_${CURRENT_TARGET}} 
            WORKING_DIRECTORY ${CURRENT_BUILD_DIR}
            OUTPUT_FILE ${CURRENT_BUILD_DIR}/build_log.stdout.txt
            ERROR_FILE ${CURRENT_BUILD_DIR}/build_log.stderr.txt
            RESULT_VARIABLE ret
        )
        CHECK_RETURN_CODE(ret)
    endif()
    # ==============================================================
    # Run Tests
    #
    if(ret EQUAL 0)
        message(STATUS "Run tests for: ${CURRENT_TARGET}") 
        
        file(GLOB test_executables RELATIVE ${CURRENT_BIN_DIR} ${CURRENT_BIN_DIR}/*Test* ${CURRENT_BIN_DIR}/*test* ${CURRENT_BIN_DIR}/*/*Test* ${CURRENT_BIN_DIR}/*/*test*)
        list(REMOVE_DUPLICATES test_executables)
        message(STATUS "Found tests: ${test_executables}")
        foreach(test_executable ${test_executables})
            
            message(STATUS "Run ${test_executable}")
            execute_process(COMMAND ${CURRENT_BIN_DIR}/${test_executable}
                WORKING_DIRECTORY ${CURRENT_BIN_DIR}
                OUTPUT_FILE ${CURRENT_BIN_DIR}/${test_executable}.stdout.txt
                ERROR_FILE ${CURRENT_BIN_DIR}/${test_executable}.stderr.txt
                RESULT_VARIABLE ret
            )
            if(ret EQUAL 0)
                message(STATUS "> ${test_executable} PASSED")
            else()
                message(STATUS "> ${test_executable} FAILED")
            endif()
            
        endforeach(test_executable)
    endif()
    
endforeach(CURRENT_TARGET) 

# ==============================================================
# Make report
#
message(STATUS "------------------------------------------------------")