cmake_minimum_required(VERSION 3.14)
project(alloc-test)

# ------------------------------------------------------------------------------
# Policies
# ------------------------------------------------------------------------------
cmake_policy(SET CMP0028 NEW) # Double colon in target name means ALIAS or
                              # IMPORTED target
cmake_policy(SET CMP0060 NEW) # Link libraries by full path even in implicit
                              # directories
cmake_policy(SET CMP0074 NEW) # find_package uses XXX_ROOT vars

# ------------------------------------------------------------------------------
# Enable testing
# ------------------------------------------------------------------------------
include(CTest)
enable_testing()

# ------------------------------------------------------------------------------
# Set C++ standard to C++17
# ------------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)

# ------------------------------------------------------------------------------
# Default to Release build type
# ------------------------------------------------------------------------------
#set(CMAKE_BUILD_TYPE Debug)

# ------------------------------------------------------------------------------
# cmake module path for find_package
# ------------------------------------------------------------------------------
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

# ------------------------------------------------------------------------------
# Setup Output directories (all binaries in bin/ change libs to lib/ if you
# want)
# ------------------------------------------------------------------------------
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY
    ${PROJECT_BINARY_DIR}/bin
    CACHE PATH "Single Directory for all Executables."
)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY
    ${PROJECT_BINARY_DIR}/bin
    CACHE PATH "Single Directory for all Libraries"
)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY
    ${PROJECT_BINARY_DIR}/bin
    CACHE PATH "Single Directory for all static libraries."
)

# ------------------------------------------------------------------------------
# Set choice variables
# ------------------------------------------------------------------------------
set(NB_THREADS
    16
    CACHE STRING "Number of threads to test with"
)
set(MAX_ITEMS
    25
    CACHE STRING "Maximum number of items in temrs of bit shifts (1 << X)"
)
configure_file(
  ${PROJECT_SOURCE_DIR}/src/config.hpp.in ${PROJECT_SOURCE_DIR}/src/config.hpp
)

# ------------------------------------------------------------------------------
# Set allocators to test, set the use of fancy pointers for new/delete
# allocators
# ------------------------------------------------------------------------------
option(ALLOC_TEST_ALL FALSE)
option(ALLOC_TEST_HWMALLOC FALSE)
option(ALLOC_TEST_TFMALLOC FALSE) # TF := Test of Fancy
option(ALLOC_TEST_JEMALLOC FALSE)
option(ALLOC_TEST_MIMALLOC FALSE)
option(ALLOC_TEST_TCMALLOC FALSE)

if (ALLOC_TEST_ALL)
    option(ALLOC_TEST_HWMALLOC TRUE)
    option(ALLOC_TEST_TFMALLOC TRUE) # TF := Test of Fancy
    option(ALLOC_TEST_JEMALLOC TRUE)
    option(ALLOC_TEST_MIMALLOC TRUE)
    option(ALLOC_TEST_TCMALLOC TRUE)
endif()

option(NEW_DELETE_WITH_FANCY_POINTERS TRUE)

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

# ------------------------------------------------------------------------------
# function to create a test from source files and link the requested allocator
# ------------------------------------------------------------------------------
function(make_allocator_app alloc_name)
  # boilerplate to parse arguments
  set(options NO_FIND_PACKAGE FANCY)
  set(one_value_args TEXTHINT)
  set(multi_value_args LINKLIBS)
  cmake_parse_arguments(
    ALLOC_OPTION "${options}" "${one_value_args}" "${multi_value_args}" ${ARGN}
  )

  if(NOT ALLOC_OPTION_NO_FIND_PACKAGE)
    find_package(${alloc_name} QUIET)
  else()
    set(${alloc_name}_FOUND TRUE)
  endif()
  string(TOUPPER ${alloc_name} UPPER_NAME)
  string(TOLOWER ${alloc_name} LOWER_NAME)

  if(ALLOC_OPTION_FANCY)
    set(TARGET_NAME allocator_tester_${LOWER_NAME}_fancy)
  else()
    set(TARGET_NAME allocator_tester_${LOWER_NAME})
  endif()

  if(${UPPER_NAME}_FOUND OR ${alloc_name}_FOUND)
    message(STATUS "${alloc_name} found : ${ALLOC_OPTION_TEXTHINT}")

    # Add the executable target
    add_executable(
      ${TARGET_NAME} src/test_common.cpp
                                     src/allocator_tester.cpp
    )
    target_include_directories(${TARGET_NAME} PUBLIC src)
    target_link_libraries(
      ${TARGET_NAME} PUBLIC ${${UPPER_NAME}_LIBRARY}
                                            ${ALLOC_OPTION_LINKLIBS}
    )
    target_compile_definitions(
      ${TARGET_NAME} PUBLIC ALLOC_TEST_${UPPER_NAME}
    )

    if(ALLOC_OPTION_FANCY)
      target_compile_definitions(
        ${TARGET_NAME} PUBLIC NEW_DELETE_WITH_FANCY_POINTERS
      )
      set(output_path ${PRINT_OUTPUT_DIR}/test_${LOWER_NAME}_fancy.txt)
    else()
      set(output_path ${PRINT_OUTPUT_DIR}/test_${LOWER_NAME}.txt)
    endif()

    # create a `ctest` test that executes the allocator tester note that ctest
    # doesn't use a shell, so we can't redirect the output to a file using the
    # usual '>' operator
    add_test(NAME ${TARGET_NAME}
             COMMAND "$<TARGET_FILE:${TARGET_NAME}>"
    )

    # create a custom target that executes the allocator tester as above, and
    # redirects the output to a file. Building this target will first build the
    # binary and then execute the test
    add_custom_target(
      print_${TARGET_NAME}
      # the target must be compiled before this test is run
      DEPENDS $<TARGET_FILE:${TARGET_NAME}>
      COMMAND $<TARGET_FILE:${TARGET_NAME}> > "${output_path}"
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      COMMENT
        "Writing output of ${TARGET_NAME} to ${output_path}"
    )

    # append the same command to the "print_all" pseudo target
    add_custom_command(
      OUTPUT print_all_dependencies
      APPEND COMMAND_EXPAND_LISTS
      COMMENT
        "Sending output of ${TARGET_NAME} to ${output_path}"
      COMMAND
        echo
        "Writing output of ${TARGET_NAME} to ${output_path}"
      COMMAND $<TARGET_FILE:${TARGET_NAME}> > "${output_path}"
    )

  else()
    message(STATUS "${alloc_name} was not found, skipping build")
  endif()
endfunction()

# ------------------------------------------------------------------------------
# create a pseudo target that will run ALL our print_xxx tests sequentially
# ------------------------------------------------------------------------------
add_custom_target(print_all DEPENDS print_all_dependencies)
add_custom_command(
  OUTPUT print_all_dependencies COMMAND echo "Running all print_xxx tests"
)
# print_all_dependencies is not actually generated so set it to symbolic
set_source_files_properties(print_all_dependencies PROPERTIES SYMBOLIC "true")

# ------------------------------------------------------------------------------
# hwmalloc
# ------------------------------------------------------------------------------
if(ALLOC_TEST_ALL OR ALLOC_TEST_HWMALLOC)

  if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/extern/hwmalloc)
    message(STATUS "Using hwmalloc in source tree")
    # use the source in this directory
    set(FETCHCONTENT_SOURCE_DIR_HWMALLOC ${CMAKE_CURRENT_SOURCE_DIR}/extern/hwmalloc)
    # don't change branches, or pull
    set(FETCHCONTENT_UPDATES_DISCONNECTED_HWMALLOC ON)
  endif()

  if(NOT _hwmalloc_already_fetched)
    set(_hwmalloc_repository "https://github.com/boeschf/hwmalloc.git")
    set(_hwmalloc_tag "refactoring2")
    message(
      STATUS "Fetching HWMALLOC ${_hwmalloc_tag} from ${_hwmalloc_repository}"
    )

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
    set(HWMALLOC_ENABLE_DEVICE  FALSE CACHE BOOL "")
    set(HWMALLOC_ENABLE_LOGGING FALSE CACHE BOOL "")
    set(HWMALLOC_DEVICE_RUNTIME 0     CACHE STRING "")

    set(_hwmalloc_already_fetched
        ON
        CACHE INTERNAL ""
    )

  endif()

  # create hwmalloc library
  add_subdirectory(${hwmalloc_SOURCE_DIR} ${hwmalloc_BINARY_DIR})

  # find Boost library
  find_package(Boost REQUIRED)

  # create the executable and link hwmalloc library to it
  make_allocator_app(hwmalloc NO_FIND_PACKAGE LINKLIBS hwmalloc Boost::boost)

endif()

# ------------------------------------------------------------------------------
# tfmalloc
# ------------------------------------------------------------------------------
if(ALLOC_TEST_ALL OR ALLOC_TEST_TFMALLOC)
  make_allocator_app(tfmalloc NO_FIND_PACKAGE)
endif()

# ------------------------------------------------------------------------------
# jemalloc
# ------------------------------------------------------------------------------
if(ALLOC_TEST_ALL OR ALLOC_TEST_JEMALLOC)
  make_allocator_app(
    Jemalloc 
    TEXTHINT "use MALLOC_CONF=stats_print:true bin/binary"
  )
  if (NEW_DELETE_WITH_FANCY_POINTERS)
    make_allocator_app(
      Jemalloc 
      TEXTHINT "use MALLOC_CONF=stats_print:true bin/binary"
      FANCY
    )
  endif()
endif()

# ------------------------------------------------------------------------------
# mimalloc
# ------------------------------------------------------------------------------
if(ALLOC_TEST_ALL OR ALLOC_TEST_MIMALLOC)
  make_allocator_app(
    mimalloc TEXTHINT "use env MIMALLOC_SHOW_STATS=1 bin/binary"
  )
  if (NEW_DELETE_WITH_FANCY_POINTERS)
    make_allocator_app(
      mimalloc TEXTHINT "use env MIMALLOC_SHOW_STATS=1 bin/binary"
      FANCY
    )
  endif()
endif()

# ------------------------------------------------------------------------------
# tcmalloc
# ------------------------------------------------------------------------------
if(ALLOC_TEST_ALL OR ALLOC_TEST_TCMALLOC)
  make_allocator_app(Tcmalloc)
  if (NEW_DELETE_WITH_FANCY_POINTERS)
    make_allocator_app(Tcmalloc FANCY)
  endif()
endif()

# create a psuedo target that executes all tests
add_custom_target(
  generate_plot_data DEPENDS print_hwmalloc print_tfmalloc print_jemalloc
                             print_mimalloc print_tcmalloc
)
