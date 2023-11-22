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

# Maximum number of threads
set(ALLOC_TEST_MAX_NUM_THREADS 4 CACHE STRING "Number of threads to test with")

# Memory access type
set(ALLOC_TEST_MEMORY_ACCESS "full" CACHE STRING "Choose the type of memory access.")
set_property(CACHE ALLOC_TEST_MEMORY_ACCESS PROPERTY STRINGS "none" "single" "full" "check")

# Options for building different tests
option(ALLOC_TEST_BUILD_ALL_TESTS "Build tests for all allocators" OFF)
cmake_dependent_option(
    ALLOC_TEST_BUILD_WITH_STDMALLOC "Build with std::malloc" OFF
    "NOT ALLOC_TEST_BUILD_ALL_TESTS" ON)
cmake_dependent_option(
    ALLOC_TEST_BUILD_WITH_PMRMALLOC "Build with std::pmr::polymorphic_allocator" OFF
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
cmake_dependent_option(
    ALLOC_TEST_BUILD_WITH_PMIMALLOC "Build with pmimalloc" OFF
    "NOT ALLOC_TEST_BUILD_ALL_TESTS" ON)
cmake_dependent_option(
    ALLOC_TEST_BUILD_WITH_TSPUMALLOC "Build with UMPIRE" OFF
    "NOT ALLOC_TEST_BUILD_ALL_TESTS" ON)
cmake_dependent_option(
    ALLOC_TEST_BUILD_WITH_HOARDMALLOC "Build with HOARD" OFF
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

# ------------------------------------------------------------------------------
# setup plot directory/options
# ------------------------------------------------------------------------------
set(PRINT_OUTPUT_DIR
    ${PROJECT_BINARY_DIR}/plot
    CACHE PATH "Directory for plot scripts"
)

# if plot/output directory doesn't exist, create it
if(NOT EXISTS ${PRINT_OUTPUT_DIR})
  file(MAKE_DIRECTORY ${PRINT_OUTPUT_DIR})
endif()

# create a symlink to plot.ipynb in the build directory
if(NOT EXISTS ${PRINT_OUTPUT_DIR}/plot.ipynb)
  execute_process(
    COMMAND
      ${CMAKE_COMMAND} -E create_symlink ${PROJECT_SOURCE_DIR}/plot/plot.ipynb
      ${PRINT_OUTPUT_DIR}/plot.ipynb
  )
endif()


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

    set(output_path ${PRINT_OUTPUT_DIR}/test_${alloc_name}.txt)

    # create a custom target that executes the allocator tester as above, and
    # redirects the output to a file. Building this target will first build the
    # binary and then execute the test
    add_custom_target(
      print_${target_name}
      # the target must be compiled before this test is run
      DEPENDS $<TARGET_FILE:${target_name}>
      COMMAND $<TARGET_FILE:${target_name}> 2>&1 | tee "${output_path}"
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      COMMENT
        "Writing output of ${target_name} to ${output_path}"
    )
endfunction()


# build different executables if enabled

if (ALLOC_TEST_BUILD_WITH_STDMALLOC)
    add_custom_executable(stdmalloc)
endif()

if (ALLOC_TEST_BUILD_WITH_PMRMALLOC)
    add_custom_executable(pmrmalloc)
endif()

if (ALLOC_TEST_BUILD_WITH_FANCYSTDMALLOC)
    add_custom_executable(fancystdmalloc)
endif()

# May need to type the path to libtcmalloc.so and to path/to/tcmalloc/include/ by hand
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
    endif()

    # create hwmalloc library
    add_subdirectory(${hwmalloc_SOURCE_DIR} ${hwmalloc_BINARY_DIR})

    # find Boost library
    find_package(Boost REQUIRED)

    add_custom_executable(hwmalloc LINKLIBS hwmalloc Boost::boost)
endif()

if (ALLOC_TEST_BUILD_WITH_PMIMALLOC)

    # # create pmimalloc library
    # add_subdirectory(extern/pmimalloc)
    # if(!ALLOC_TEST_BUILD_WITH_MIMALLOC)
    #     find_package(mimalloc REQUIRED)
    # endif()
    find_library(PMIMALLOC_NONE_LIB pmimalloc_none HINTS home/nathan/build/pmimalloc/bin/libpmimalloc_none.so)

    # find mimalloc library
    # find_package(pmimalloc REQUIRED)

    add_custom_executable(pinned_mimalloc 
                    LINKLIBS 
                        ${PMIMALLOC_NONE_LIB}
                        # pmimalloc_none
                        # mimalloc
                )
    # target_compile_options(pinned_mimalloc PRIVATE -DWITH_MIMALLOC)
    # target_include_directories(pinned_mimalloc PRIVATE extern/pmimalloc/include)

endif()

if (ALLOC_TEST_BUILD_WITH_TSPUMALLOC)
    find_package(umpire REQUIRED)
    add_custom_executable(tspumalloc
        #DEACTIVATE_BUILTIN_ALLOC
        LINKLIBS umpire)
endif()

if (ALLOC_TEST_BUILD_WITH_HOARDMALLOC)
    # set(Hoard_DIR "/home/nathan/src/alloc-test-new/extern/Hoard/src")
    find_library(HOARD_LIB NAMES hoard PATHS /home/nathan/src/alloc-test-new/extern/Hoard/src)
    # find_package(Hoard REQUIRED)
    add_custom_executable(hoardmalloc DEACTIVATE_BUILTIN_ALLOC LINKLIBS ${HOARD_LIB})
endif()
