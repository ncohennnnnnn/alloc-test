cmake_minimum_required(VERSION 3.0)
project(alloc-tools)

#------------------------------------------------------------------------------
# Policies
#------------------------------------------------------------------------------
cmake_policy(SET CMP0028 NEW) # Double colon in target name means ALIAS or IMPORTED target
cmake_policy(SET CMP0060 NEW) # Link libraries by full path even in implicit directories
cmake_policy(SET CMP0074 NEW) # find_package uses XXX_ROOT vars

#------------------------------------------------------------------------------
# Set C++ standard to C++11
#------------------------------------------------------------------------------
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
# cmake module path for find_package
#------------------------------------------------------------------------------
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

#------------------------------------------------------------------------------
# jemalloc
#------------------------------------------------------------------------------
find_package(Jemalloc QUIET)

if(JEMALLOC_FOUND)
    message(STATUS "Jemalloc found")
    message(STATUS "use MALLOC_CONF=stats_print:true bin/binary")

    # Add the executable target
    add_executable(allocator_tester_jemalloc
      src/test_common.cpp
      src/allocator_tester.cpp
    )
    target_include_directories(allocator_tester_jemalloc PUBLIC  src)
    target_link_libraries(allocator_tester_jemalloc PUBLIC ${JEMALLOC_LIBRARY})
else()
    message(STATUS "Jemalloc was not found")
  return()
endif()

#------------------------------------------------------------------------------
# mimalloc
#------------------------------------------------------------------------------
find_package(mimalloc QUIET)

if(mimalloc_FOUND)
    message(STATUS "mimalloc found")
    message(STATUS "use env MIMALLOC_SHOW_STATS=1 bin/binary")

    # Add the executable target
    add_executable(allocator_tester_mimalloc
      src/test_common.cpp
      src/allocator_tester.cpp
    )
    target_include_directories(allocator_tester_mimalloc PUBLIC src)
    target_link_libraries(allocator_tester_mimalloc PUBLIC mimalloc)
else()
    message(STATUS "mimalloc was not found")
  return()
endif()

#------------------------------------------------------------------------------
# tcmalloc
#------------------------------------------------------------------------------
find_package(Tcmalloc QUIET)

if(Tcmalloc_FOUND)
    message(STATUS "tcmalloc found")

    # Add the executable target
    add_executable(allocator_tester_tcmalloc
      src/test_common.cpp
      src/allocator_tester.cpp
    )
    target_include_directories(allocator_tester_tcmalloc PUBLIC src)
    target_link_libraries(allocator_tester_tcmalloc PUBLIC ${Tcmalloc_LIBRARY})
else()
    message(STATUS "tcmalloc was not found")
  return()
endif()
