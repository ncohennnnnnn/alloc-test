cmake_minimum_required(VERSION 3.19)
include(CMakeDependentOption)

project(alloc-test)

# Read version from file
file(STRINGS "VERSION" PROJECT_VERSION)

# Extract major, minor, and patch parts
string(REGEX MATCH "([0-9]+)\\.([0-9]+)\\.(.*)" _ ${PROJECT_VERSION})
set(MAJOR_VERSION ${CMAKE_MATCH_1})
set(MINOR_VERSION ${CMAKE_MATCH_2})
set(PATCH_VERSION ${CMAKE_MATCH_3})

# Set the project version
project(alloc-test VERSION ${PROJECT_VERSION})

# cmake module path for find_package
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

# Set the output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Set the library and archive output directories
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Set the C++ standard to 17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# maximum number of threads
set(ALLOC_TEST_MAX_NUM_THREADS 4 CACHE STRING "Number of threads to test with")

# Options for building different tests
option(ALLOC_TEST_BUILD_ALL_TESTS "Build tests for all allocators" OFF)
cmake_dependent_option(
    ALLOC_TEST_BUILD_WITH_STDMALLOC "Build with std::malloc" OFF
    "NOT ALLOC_TEST_BUILD_ALL_TESTS" ON)
cmake_dependent_option(
    ALLOC_TEST_BUILD_WITH_FANCYSTDMALLOC "Build with std::malloc using fancy pointers" OFF
    "NOT ALLOC_TEST_BUILD_ALL_TESTS" ON)
cmake_dependent_option(
    ALLOC_TEST_BUILD_WITH_TCMALLOC "Build with tcmalloc" OFF
    "NOT ALLOC_TEST_BUILD_ALL_TESTS" ON)
cmake_dependent_option(
    ALLOC_TEST_BUILD_WITH_MIMALLOC "Build with mimalloc" OFF
    "NOT ALLOC_TEST_BUILD_ALL_TESTS" ON)
cmake_dependent_option(
    ALLOC_TEST_BUILD_WITH_JEMALLOC "Build with jemalloc" OFF
    "NOT ALLOC_TEST_BUILD_ALL_TESTS" ON)
cmake_dependent_option(
    ALLOC_TEST_BUILD_WITH_HWMALLOC "Build with hwmalloc" OFF
    "NOT ALLOC_TEST_BUILD_ALL_TESTS" ON)

# Add your source files
set(SOURCES src/test.cpp)

# Common compiler options
set(COMMON_OPTIONS
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
)

# Position Independent Code (PIC) option
set(POSITION_INDEPENDENT_CODE ON)

# create config file
configure_file(
    ${PROJECT_SOURCE_DIR}/cmake/config.hpp.in ${PROJECT_BINARY_DIR}/include/config.hpp
)

# required dependencies
find_package(fmt REQUIRED)
find_package(HWLOC REQUIRED)

# create a shared library containing common utilities
add_library(common SHARED src/thread_affinity.cpp src/task_group.cpp src/get_rss.cpp)
set_property(TARGET common PROPERTY POSITION_INDEPENDENT_CODE ${POSITION_INDEPENDENT_CODE})
target_link_libraries(common PRIVATE HWLOC::libhwloc)
target_link_libraries(common PRIVATE fmt::fmt-header-only)
target_compile_options(common PRIVATE ${COMMON_OPTIONS})
target_include_directories(common PRIVATE
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>)

# Function to compile executables
function(add_custom_executable alloc_name)
    cmake_parse_arguments(ALLOC_OPTION "DEACTIVATE_BUILTIN_ALLOC" "" "LINKLIBS;LINKDIRS;INCLUDEDIRS" ${ARGN})

    string(TOUPPER ${alloc_name} UPPER_NAME)
    set(alloc_flag ALLOC_TEST_${UPPER_NAME})
    set(target_name ${alloc_name}_test)

    add_executable(${target_name} ${SOURCES})
    target_compile_definitions(${target_name} PRIVATE ${alloc_flag})
    set_property(TARGET ${target_name} PROPERTY POSITION_INDEPENDENT_CODE ${POSITION_INDEPENDENT_CODE})
    target_link_directories(${target_name} PRIVATE ${ALLOC_OPTION_LINKDIRS})
    target_link_libraries(${target_name} PRIVATE ${ALLOC_OPTION_LINKLIBS})
    target_link_libraries(${target_name} PRIVATE common)
    target_link_libraries(${target_name} PRIVATE fmt::fmt-header-only)

    target_compile_options(${target_name} PRIVATE ${COMMON_OPTIONS})
    if(ALLOC_OPTION_DEACTIVATE_BUILTIN_ALLOC)
        target_compile_options(${target_name} PRIVATE -fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free)
    endif()

    target_include_directories(${target_name} PRIVATE ${ALLOC_OPTION_INCLUDEDIRS})
    target_include_directories(${target_name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_include_directories(${target_name} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/include)
endfunction()


# build different executables if enabled

if (ALLOC_TEST_BUILD_WITH_STDMALLOC)
    add_custom_executable(stdmalloc)
endif()

if (ALLOC_TEST_BUILD_WITH_FANCYSTDMALLOC)
    add_custom_executable(fancystdmalloc)
endif()

if (ALLOC_TEST_BUILD_WITH_TCMALLOC)
    find_package(Tcmalloc REQUIRED)
    add_custom_executable(tcmalloc DEACTIVATE_BUILTIN_ALLOC LINKLIBS ${Tcmalloc_LIBRARY})
endif()

if (ALLOC_TEST_BUILD_WITH_MIMALLOC)
    find_package(mimalloc REQUIRED)
    add_custom_executable(mimalloc DEACTIVATE_BUILTIN_ALLOC LINKLIBS mimalloc)
endif()

if (ALLOC_TEST_BUILD_WITH_JEMALLOC)
    #find_package(Jemalloc REQUIRED)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(JEMALLOC jemalloc)
    pkg_search_module(JEMALLOC REQUIRED jemalloc)
    message(STATUS ${JEMALLOC_LIBRARY_DIRS})
    add_custom_executable(jemalloc
        DEACTIVATE_BUILTIN_ALLOC
        INCLUDEDIRS ${JEMALLOC_INCLUDE_DIRS}
        LINKDIRS ${JEMALLOC_LIBRARY_DIRS}
        LINKLIBS ${JEMALLOC_LIBRARIES})
endif()

if (ALLOC_TEST_BUILD_WITH_HWMALLOC)
    if(NOT _hwmalloc_already_fetched)
        set(_hwmalloc_repository "https://github.com/boeschf/hwmalloc.git")
        set(_hwmalloc_tag "refactoring2")
        message(STATUS "Fetching HWMALLOC ${_hwmalloc_tag} from ${_hwmalloc_repository}")

        include(FetchContent)
        fetchcontent_declare(
          hwmalloc
          GIT_REPOSITORY ${_hwmalloc_repository}
          GIT_TAG ${_hwmalloc_tag}
        )
        fetchcontent_populate(hwmalloc)

        # -------------------------------------------------------
        # Compile hwmalloc without NUMA, Boost, logging or device
        # -------------------------------------------------------
        set(HWMALLOC_ENABLE_DEVICE FALSE)
        set(HWMALLOC_ENABLE_LOGGING FALSE)
        set(HWMALLOC_DEVICE_RUNTIME 0)

        set(_hwmalloc_already_fetched ON CACHE INTERNAL "")

        #configure_file(
        #  ${CMAKE_CURRENT_BINARY_DIR}/_deps/hwmalloc-src/cmake/config.hpp.in
        #  ${CMAKE_CURRENT_BINARY_DIR}/_deps/hwmalloc-src/cmake/config.hpp
        #)
    endif()

    # create hwmalloc library
    add_subdirectory(${hwmalloc_SOURCE_DIR} ${hwmalloc_BINARY_DIR})

    # find Boost library
    find_package(Boost REQUIRED)

    add_custom_executable(hwmalloc LINKLIBS hwmalloc Boost::boost)
endif()

