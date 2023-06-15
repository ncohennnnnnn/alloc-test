cmake_minimum_required(VERSION 3.0)
project(alloc-tools)

# Set C++ standard to C++11
set(CMAKE_CXX_STANDARD 17)

#------------------------------------------------------------------------------
# Setup Output directories
#------------------------------------------------------------------------------
SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY
    ${PROJECT_BINARY_DIR}/bin CACHE PATH "Single Directory for all Executables."
)
SET (CMAKE_LIBRARY_OUTPUT_DIRECTORY
    ${PROJECT_BINARY_DIR}/bin CACHE PATH "Single Directory for all Libraries"
)
SET (CMAKE_ARCHIVE_OUTPUT_DIRECTORY
    ${PROJECT_BINARY_DIR}/bin CACHE PATH "Single Directory for all static libraries."
)

#------------------------------------------------------------------------------
# jemalloc
#------------------------------------------------------------------------------
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
find_package(Jemalloc QUIET)

if(JEMALLOC_FOUND)
    message(STATUS "Jemalloc found")
    message(STATUS "use MALLOC_CONF=stats_print:true bin/binary")
else()
    message(STATUS "Jemalloc was not found")
  return()
endif()


if (JEMALLOC_FOUND) 
  # Add the executable target
  add_executable(allocator_tester_jemalloc
    src/test_common.cpp
    src/allocator_tester.cpp
  )
  target_include_directories(allocator_tester_jemalloc PRIVATE src)
  target_link_libraries(allocator_tester_jemalloc ${JEMALLOC_LIBRARY})
endif()

