cmake_minimum_required (VERSION 3.2)

# ==============================================================
# Usage
#   -D_TARGET=<target name>
#   -D_CONFIGURATION=[Debug, Release, All]
#   -D_MT=[ON/OFF] Multithreaded build
# Supported targets:
#   vc12x32  - MSVC_2013 x32
#   vc14x32  - MSVC_2015 x32
#   vc12x64  - MSVC_2013 x64
#   vc14x64  - MSVC_2015 x64
#   mingw    - MinGW
#   gcc      - Unix GCC 
#   clang    - clang (specify environment variable LLVM_ROOT in Windows)
#   android  - android toolchain (clang) (specify environment variable ANDROID_TOOLCHAIN with path of standalone toolchain)
#   all      - enable all listed (filtered by operating system)

# ==============================================================
# Global variables
#

find_program(CLANG_FOUND clang)
find_program(GCC_FOUND gcc)

if(WIN32)
    if(GCC_FOUND)
        list(APPEND SUPPORTED_TARGETS mingw)
    endif()
    if(CLANG_FOUND)
        list(APPEND SUPPORTED_TARGETS clang)
    endif()
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
    if(GCC_FOUND)
        list(APPEND SUPPORTED_TARGETS gcc)
    endif()
    if(CLANG_FOUND)
        list(APPEND SUPPORTED_TARGETS clang)
    endif()
endif()

if(NOT ANDROID_TOOLCHAIN)
    set(ANDROID_TOOLCHAIN $ENV{ANDROID_TOOLCHAIN})
endif()
if(ANDROID_TOOLCHAIN)
    find_path(ANDROID_FOUND "AndroidVersion.txt" ${ANDROID_TOOLCHAIN})
    if(ANDROID_FOUND)
        list(APPEND SUPPORTED_TARGETS android)
    endif()
endif()

set(GENERATOR_mingw "MinGW Makefiles")
set(GENERATOR_vc12x32 "Visual Studio 12 2013")
set(GENERATOR_vc12x64 "Visual Studio 12 2013 Win64")
set(GENERATOR_vc14x32 "Visual Studio 14 2015")
set(GENERATOR_vc14x64 "Visual Studio 14 2015 Win64")
set(GENERATOR_gcc "Unix Makefiles")
if(WIN32)
    set(GENERATOR_clang "MinGW Makefiles")
    set(GENERATOR_android "MinGW Makefiles")
else()
    set(GENERATOR_clang "Unix Makefiles")
    set(GENERATOR_android "Unix Makefiles")
endif()

set(TOOLCHAIN_clang "cmake/clang.toolchain.cmake")
set(TOOLCHAIN_android "cmake/android.toolchain.cmake")

list(APPEND MAKE_ARGUMENTS_mingw "")
list(APPEND MAKE_ARGUMENTS_vc12x32 "")
list(APPEND MAKE_ARGUMENTS_vc12x64 "")
list(APPEND MAKE_ARGUMENTS_vc14x32 "")
list(APPEND MAKE_ARGUMENTS_vc14x64 "")
list(APPEND MAKE_ARGUMENTS_gcc "")
list(APPEND MAKE_ARGUMENTS_clang "")
list(APPEND MAKE_ARGUMENTS_android "")

if(_MT)
    list(APPEND MAKE_ARGUMENTS_mingw "-j")
    list(APPEND MAKE_ARGUMENTS_gcc "-j")
    list(APPEND MAKE_ARGUMENTS_clang "-j")
    list(APPEND MAKE_ARGUMENTS_android "-j")
endif()

# ==============================================================
# Input argumets
#

set(error FALSE)
if(NOT DEFINED _TARGET)
    set(_TARGET "all")
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

if(NOT DEFINED _WORKSPACE)
    set(_WORKSPACE ${_SOURCE_DIR}/cmake_workspace)
endif()

if(NOT DEFINED _BUILD_DIR)
    set(_BUILD_DIR ${_WORKSPACE}/build)
endif()
file(MAKE_DIRECTORY ${_BUILD_DIR})

if(NOT DEFINED _BIN_DIR)
    set(_BIN_DIR ${_WORKSPACE}/bin)
endif()
file(MAKE_DIRECTORY ${_BIN_DIR})

if(_TARGET STREQUAL all)
    set(all_build_targers ${SUPPORTED_TARGETS})
else()
    set(all_build_targers ${_TARGET})
endif()


list(APPEND possible_configurations "Debug")
list(APPEND possible_configurations "Release")
set(CMAKE_CONFIGURATION_TYPES ${possible_configurations})
if(_CONFIGURATION)
    list(FIND possible_configurations ${_CONFIGURATION} idx)
    if(idx LESS 0)
        message(FATAL_ERROR "Invalid configuration value. Possible configurations: ${possible_configurations}")
    endif()
    list(APPEND all_configuratins ${_CONFIGURATION})
else()
    set(all_configuratins ${possible_configurations})
endif()
unset(possible_configurations)


# ==============================================================
# Make all targets
#

set(REPORT_FILE ${_WORKSPACE}/report.txt)
file(REMOVE ${REPORT_FILE})

macro(LOGGED_MESSAGE _kind _message)
    file(APPEND ${REPORT_FILE} "${_message}\n")
    message(${_kind} ${_message})
endmacro()

macro(CHECK_RETURN_CODE _ret_code)
    if(${_ret_code} EQUAL 0)
        LOGGED_MESSAGE(STATUS OK)
    else()
        LOGGED_MESSAGE(STATUS "ERROR! Return code ${${_ret_code}}")
        set(OVERALL_STATUS OFF)
    endif()
endmacro()

set(OVERALL_STATUS ON)

message(STATUS "Building for the following targets: ${all_build_targers}")
message(STATUS "Building for the following configurations: ${all_configuratins}")
file(APPEND ${REPORT_FILE} "Building for the following targets: ${all_build_targers}\n")
file(APPEND ${REPORT_FILE} "Building for the following configurations: ${all_configuratins}\n")

foreach(CURRENT_TARGET ${all_build_targers})
    LOGGED_MESSAGE(STATUS "======================================================")
    
    foreach(CURRENT_CONFIGURATION ${all_configuratins})
        LOGGED_MESSAGE(STATUS "------------------------------------------------------")
    
        set(CURRENT_BUILD_DIR ${_BUILD_DIR}/${CURRENT_TARGET}/${CURRENT_CONFIGURATION})
        set(CURRENT_BIN_DIR ${_BIN_DIR}/${CURRENT_TARGET}/${CURRENT_CONFIGURATION})
        
        file(REMOVE_RECURSE ${CURRENT_BUILD_DIR} ${CURRENT_BIN_DIR})
        file(MAKE_DIRECTORY ${CURRENT_BUILD_DIR} ${CURRENT_BIN_DIR}) 
        
        # ==============================================================
        # Configure
        #
        LOGGED_MESSAGE(STATUS "Configuring for: ${CURRENT_TARGET} / ${CURRENT_CONFIGURATION}") 
        
        unset(CUSTOM_TOOLCHAIN_ARG)
        if(DEFINED TOOLCHAIN_${CURRENT_TARGET})
            set(CUSTOM_TOOLCHAIN_ARG "-DCMAKE_TOOLCHAIN_FILE=${CMAKE_SOURCE_DIR}/${TOOLCHAIN_${CURRENT_TARGET}}")
            LOGGED_MESSAGE(STATUS "Using toolchain ${TOOLCHAIN_${CURRENT_TARGET}}")
        endif()
        
        execute_process(COMMAND cmake "-G${GENERATOR_${CURRENT_TARGET}}" ${CUSTOM_TOOLCHAIN_ARG} -DBIN_OUTPUT_DIR=${CURRENT_BIN_DIR} ${_SOURCE_DIR} -DCMAKE_BUILD_TYPE=${CURRENT_CONFIGURATION}
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
            LOGGED_MESSAGE(STATUS "Building: ${CURRENT_TARGET}") 
            
            # Find solution files
            unset(msvc_configuration)
            if(CURRENT_TARGET MATCHES vc*)
                set(msvc_configuration "/property:Configuration=${CURRENT_CONFIGURATION}")
            endif() 
            execute_process(COMMAND cmake --build ${CURRENT_BUILD_DIR} -- ${msvc_configuration} ${MAKE_ARGUMENTS_${CURRENT_TARGET}}
                #WORKING_DIRECTORY ${CURRENT_BUILD_DIR}
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
            LOGGED_MESSAGE(STATUS "Run tests for: ${CURRENT_TARGET}") 
            
            if(CURRENT_TARGET STREQUAL "android")
                LOGGED_MESSAGE(STATUS "SKIPPING")
                continue()
            endif()
            
            file(GLOB test_executables RELATIVE ${CURRENT_BIN_DIR} ${CURRENT_BIN_DIR}/*[Tt]est* ${CURRENT_BIN_DIR}/*/*[Tt]est*)
            list(REMOVE_DUPLICATES test_executables)
            # remove libs
            foreach(f ${test_executables})
                if(${f} MATCHES ".*\.(a|lib|so|dll|pdb|ilk)$")
                    list(REMOVE_ITEM test_executables ${f})
                endif()            
            endforeach()
            
            LOGGED_MESSAGE(STATUS "Found tests: ${test_executables}")
            foreach(test_executable ${test_executables})
                
                LOGGED_MESSAGE(STATUS "Run ${test_executable}")
                execute_process(COMMAND ${CURRENT_BIN_DIR}/${test_executable} "--gtest_output=xml:gtest_results.xml"
                    WORKING_DIRECTORY ${CURRENT_BIN_DIR}
                    OUTPUT_FILE ${CURRENT_BIN_DIR}/${test_executable}.stdout.txt
                    ERROR_FILE ${CURRENT_BIN_DIR}/${test_executable}.stderr.txt
                    RESULT_VARIABLE ret
                )
                if(ret EQUAL 0)
                    LOGGED_MESSAGE(STATUS "> ${test_executable} PASSED")
                else()
                    LOGGED_MESSAGE(STATUS "> ${test_executable} FAILED")
                    set(OVERALL_STATUS OFF)
                endif()
                
            endforeach(test_executable)
        endif()
    endforeach(CURRENT_CONFIGURATION)
    LOGGED_MESSAGE(STATUS "------------------------------------------------------")
endforeach(CURRENT_TARGET) 
LOGGED_MESSAGE(STATUS "======================================================")

if(NOT OVERALL_STATUS)
    message(FATAL_ERROR "Some tests have failed!")
else()
    message(STATUS "All tests are passed!")
endif()
LOGGED_MESSAGE(STATUS "======================================================")
