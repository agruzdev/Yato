cmake_minimum_required(VERSION 3.0)

if(DEFINED TOOLCHAIN_USED)
    return()
endif()
set(TOOLCHAIN_USED TRUE)

if(CMAKE_TOOLCHAIN_FILE)
    # touch toolchain variable to suppress "unused variable" warning
endif()

if(NOT ANDROID_TOOLCHAIN)
    set(ANDROID_TOOLCHAIN $ENV{ANDROID_TOOLCHAIN})
endif()
if(NOT ANDROID_TOOLCHAIN)
    message(FATAL_ERROR "Failed to find android toolchian. Please set variable ANDROID_TOOLCHAIN to correct toolchain path")
endif()
message(STATUS "ANDROID_TOOLCHAIN=${ANDROID_TOOLCHAIN}")

set(CMAKE_SYSTEM_NAME Generic)

if(WIN32)
    set(OS_SUFFIX_COMPILER ".cmd")
    set(OS_SUFFIX_OTHER ".exe")
else()
    set(OS_SUFFIX_COMPILER "")
    set(OS_SUFFIX_OTHER "")
endif()


file(GLOB toolchain_c_compiler   ${ANDROID_TOOLCHAIN}/bin/*-android-clang${OS_SUFFIX_COMPILER})
file(GLOB toolchain_cxx_compiler ${ANDROID_TOOLCHAIN}/bin/*-android-clang++${OS_SUFFIX_COMPILER})
file(GLOB toolchain_linker       ${ANDROID_TOOLCHAIN}/bin/*-android-ld.gold${OS_SUFFIX_OTHER})
file(GLOB toolchain_ar           ${ANDROID_TOOLCHAIN}/bin/*-android-ar${OS_SUFFIX_OTHER})
file(GLOB toolchain_nm           ${ANDROID_TOOLCHAIN}/bin/*-android-nm${OS_SUFFIX_OTHER})
file(GLOB toolchain_objcopy      ${ANDROID_TOOLCHAIN}/bin/*-android-objcopy${OS_SUFFIX_OTHER})
file(GLOB toolchain_objdump      ${ANDROID_TOOLCHAIN}/bin/*-android-objdump${OS_SUFFIX_OTHER})
file(GLOB toolchain_ranlib       ${ANDROID_TOOLCHAIN}/bin/*-android-ranlib${OS_SUFFIX_OTHER})
file(GLOB toolchain_strip        ${ANDROID_TOOLCHAIN}/bin/*-android-strip${OS_SUFFIX_OTHER})

#ToDo: Add more checks here


set(CMAKE_C_COMPILER    ${toolchain_c_compiler}   CACHE FILE "Path to program")
set(CMAKE_CXX_COMPILER  ${toolchain_cxx_compiler} CACHE FILE "Path to program")
set(CMAKE_LINKER        ${toolchain_linker}       CACHE FILE "Path to program")
set(CMAKE_NM            ${toolchain_nm}           CACHE FILE "Path to program")
set(CMAKE_AR            ${toolchain_ar}           CACHE FILE "Path to program")
set(CMAKE_OBJCOPY       ${toolchain_objcopy}      CACHE FILE "Path to program")
set(CMAKE_OBJDUMP       ${toolchain_objdump}      CACHE FILE "Path to program")
set(CMAKE_RANLIB        ${toolchain_ranlib}       CACHE FILE "Path to program")
set(CMAKE_STRIP         ${toolchain_strip}        CACHE FILE "Path to program")


set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -fpic -std=c11 "    CACHE STTRING "C flags")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpic -std=c++14 " CACHE STTRING "C++ flags")
set(CMAKE_SHARED_LINKER_FLAGS  "${CMAKE_SHARED_LINKER_FLAGS} -fPIE -pie " CACHE STTRING "shared linker flags")
set(CMAKE_EXE_LINKER_FLAGS     "${CMAKE_EXE_LINKER_FLAGS} -fPIE -pie " CACHE STTRING "exe linker flags")

set(CMAKE_C_STANDARD_LIBRARIES   "${CMAKE_C_STANDARD_LIBRARIES}  -lgcc "    CACHE STRING "Standard C Libraries")
set(CMAKE_CXX_STANDARD_LIBRARIES "${MAKE_CXX_STANDARD_LIBRARIES} -lstdc++ " CACHE STRING "Standard C++ Libraries")

set(CLANG ON)
set(ANDROID ON)
