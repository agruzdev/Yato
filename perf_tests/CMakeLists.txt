project(YatoPerfTests)

#=======================================================

include(${YATO_SOURCE_DIR}/cmake/dependency.gbenchmark.cmake)

#=======================================================

file(GLOB_RECURSE perf_sources ./source/*.h ./source/*.cpp)

source_group(TREE "${YATO_SOURCE_DIR}" FILES ${perf_sources})

include_directories(${YATO_INCLUDE_DIRS})
include_directories(${GBENCH_INCLUDE_DIR})
link_directories(${GBENCH_LIBRARY_DIR})


add_executable(YatoPerfTests ${perf_sources})
set_property(TARGET YatoPerfTests PROPERTY FOLDER "Tests")

foreach(lib ${GBENCH_LIBRARIES})
    target_link_libraries(YatoPerfTests ${lib})
endforeach()


