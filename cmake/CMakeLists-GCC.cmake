# GCC

# ===============================================
# Compiler flags

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wpedantic")
if(CLANG)
    if(NOT ANDROID)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -femulated-tls")
    endif()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif()

#ToDo (a.gruzdev) Temporal workaround
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-format-security")
if(NOT ANDROID)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-noexcept-type")
endif()


option(YATO_WITH_ADDRESS_SANITIZER "Enable address sanitizing" OFF)
if(YATO_WITH_ADDRESS_SANITIZER)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer -g -fsanitize=address")
endif()

