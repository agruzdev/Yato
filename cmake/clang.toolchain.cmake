cmake_minimum_required(VERSION 3.0)

if(DEFINED TOOLCHAIN_USED)
    return()
endif()
set(TOOLCHAIN_USED TRUE)

if(CMAKE_TOOLCHAIN_FILE)
    # touch toolchain variable to suppress "unused variable" warning
endif()

set(CMAKE_C_COMPILER   clang   CACHE FILE "C compiler")
set(CMAKE_CXX_COMPILER clang++ CACHE FILE "C++ compiler")

if(WIN32)
    set(target "-target x86_64-mingw32")
else()
    set(target "")
endif()

set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   ${target}" CACHE STTRING "C flags")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${target} -std=c++14 " CACHE STTRING "C++ flags")
set(CMAKE_C_STANDARD_LIBRARIES   "${CMAKE_C_STANDARD_LIBRARIES}  -lgcc "    CACHE STRING "Standard C Libraries")
set(CMAKE_CXX_STANDARD_LIBRARIES "${MAKE_CXX_STANDARD_LIBRARIES} -lstdc++ " CACHE STRING "Standard C++ Libraries")

unset(target)

set(CLANG ON)
