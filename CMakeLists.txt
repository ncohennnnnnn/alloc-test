cmake_minimum_required(VERSION 3.14)
project(alloc-test)

#------------------------------------------------------------------------------
# Policies
#------------------------------------------------------------------------------
cmake_policy(SET CMP0028 NEW) # Double colon in target name means ALIAS or IMPORTED target
cmake_policy(SET CMP0060 NEW) # Link libraries by full path even in implicit directories
cmake_policy(SET CMP0074 NEW) # find_package uses XXX_ROOT vars

#------------------------------------------------------------------------------
# Set choice variables
#------------------------------------------------------------------------------
set(NB_THREADS 16)
configure_file(${PROJECT_SOURCE_DIR}/src/config.hpp.in ${PROJECT_SOURCE_DIR}/src/config.hpp)

#------------------------------------------------------------------------------
# Set allocators to test, set the use of fancy pointers for new/delete allocators
#------------------------------------------------------------------------------
set(TEST_ALL FALSE)
set(TEST_HWMALLOC FALSE)
set(TEST_TFMALLOC FALSE) # TF := Test of Fancy
set(TEST_JEMALLOC TRUE)
set(TEST_MIMALLOC FALSE)
set(TEST_TCMALLOC FALSE)

set(NEW_DELETE_WITH_FANCY_POINTERS TRUE)

if (TEST_ALL)
    set(TEST_HWMALLOC TRUE)
    set(TEST_TFMALLOC TRUE)
    set(TEST_JEMALLOC TRUE)
    set(TEST_MIMALLOC TRUE)
    set(TEST_TCMALLOC TRUE)
endif()


#------------------------------------------------------------------------------
# Set print
#------------------------------------------------------------------------------
set(PRINT_IN_TXT_ALL FALSE)
set(PRINT_IN_TXT_HWMALLOC FALSE)
set(PRINT_IN_TXT_TFMALLOC FALSE)
set(PRINT_IN_TXT_JEMALLOC TRUE)
set(PRINT_IN_TXT_MIMALLOC TRUE)
set(PRINT_IN_TXT_TCMALLOC TRUE)

if (PRINT_IN_TXT_ALL)
    set(PRINT_IN_TXT_HWMALLOC TRUE)
    set(PRINT_IN_TXT_TFMALLOC TRUE)
    set(PRINT_IN_TXT_JEMALLOC TRUE)
    set(PRINT_IN_TXT_MIMALLOC TRUE)
    set(PRINT_IN_TXT_TCMALLOC TRUE)
endif()

if((PRINT_IN_TXT_ALL OR PRINT_IN_TXT_HWMALLOC OR PRINT_IN_TXT_TFMALLOC OR PRINT_IN_TXT_JEMALLOC OR PRINT_IN_TXT_MIMALLOC OR PRINT_IN_TXT_TCMALLOC)
  AND 
  NOT EXISTS ${PROJECT_SOURCE_DIR}/plot/output)

    file(MAKE_DIRECTORY ${PROJECT_SOURCE_DIR}/plot/output)

endif()

#------------------------------------------------------------------------------
# Set C++ standard to C++17
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
#------------------------------------------------------------------------------
# For allocators passed as a new class
#------------------------------------------------------------------------------
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# hwmalloc
#------------------------------------------------------------------------------
if(TEST_ALL OR TEST_HWMALLOC)

    set(ALLOC_TEST_HWMALLOC TRUE)

    if(NOT _hwmalloc_already_fetched)

        set(_hwmalloc_repository "https://github.com/boeschf/hwmalloc.git")
        set(_hwmalloc_tag        "refactoring2")

        message(STATUS "Fetching HWMALLOC ${_hwmalloc_tag} from ${_hwmalloc_repository}")
        
        include(FetchContent)
        FetchContent_Declare(
            hwmalloc
            GIT_REPOSITORY ${_hwmalloc_repository}
            GIT_TAG        ${_hwmalloc_tag}
        )
        FetchContent_Populate(hwmalloc)

        #------------------------------------------------------------------------------
        # Compile and build hwmalloc without NUMA, Boost, logging or device
        #------------------------------------------------------------------------------

        set(HWMALLOC_ENABLE_DEVICE FALSE)
        set(HWMALLOC_ENABLE_LOGGING FALSE)
        set(HWMALLOC_DEVICE_RUNTIME 0)

        set(_hwmalloc_already_fetched ON CACHE INTERNAL "")

        configure_file(${CMAKE_CURRENT_BINARY_DIR}/_deps/hwmalloc-src/cmake/config.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/_deps/hwmalloc-src/cmake/config.hpp)

    endif()

    #------------------------------------------------------------------------------
    # Build the alloc-test executable and link the hwmalloc library to it
    #------------------------------------------------------------------------------
    set(_hwmalloc_directory ${CMAKE_CURRENT_BINARY_DIR}/_deps/hwmalloc-src)

    add_subdirectory(${_hwmalloc_directory})

    add_executable(allocator_tester_hwmalloc
    src/test_common.cpp
    src/allocator_tester.cpp
    )

    find_package(Boost REQUIRED)

    target_compile_definitions(allocator_tester_hwmalloc PUBLIC ALLOC_TEST_HWMALLOC)

    target_link_libraries(allocator_tester_hwmalloc Boost::boost)
    target_link_libraries(allocator_tester_hwmalloc hwmalloc)

    target_include_directories(allocator_tester_hwmalloc
    PUBLIC
        src
        ${CMAKE_CURRENT_BINARY_DIR}/_deps/hwmalloc-src/include
    )

    if((PRINT_IN_TXT_ALL OR PRINT_IN_TXT_HWMALLOC) AND (TEST_ALL OR TEST_HWMALLOC))

        if(EXISTS ${PROJECT_SOURCE_DIR}/plot/output/test_hwmalloc.txt)
            message("Removing previous hwmalloc result file...")
            file(REMOVE ${PROJECT_SOURCE_DIR}/plot/output/test_hwmalloc.txt)
        endif()
        
        set(output_hwmalloc_path ${PROJECT_SOURCE_DIR}/plot/output/test_hwmalloc.txt)

        add_custom_command(
            TARGET allocator_tester_hwmalloc
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E env $<TARGET_FILE:allocator_tester_hwmalloc> > "${output_hwmalloc_path}"
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Redirecting output to ${output_hwmalloc_path}"
        )

    endif()

    set(ALLOC_TEST_HWMALLOC FALSE)
    
endif() # test of hwmalloc

#------------------------------------------------------------------------------
# tfmalloc
#------------------------------------------------------------------------------
if(TEST_ALL OR TEST_TFMALLOC)

    set(ALLOC_TEST_TFMALLOC TRUE)

    add_executable(allocator_tester_tfmalloc
    src/test_common.cpp
    src/allocator_tester.cpp
    )
    target_include_directories(allocator_tester_tfmalloc PUBLIC  src)
    target_compile_definitions(allocator_tester_tfmalloc PUBLIC ALLOC_TEST_TFMALLOC)

    if((PRINT_IN_TXT_ALL OR PRINT_IN_TXT_TFMALLOC) AND (TEST_ALL OR TEST_TFMALLOC))

        if(EXISTS ${PROJECT_SOURCE_DIR}/plot/output/test_tfmalloc.txt)
            message("Removing previous tfmalloc result file...")
            file(REMOVE ${PROJECT_SOURCE_DIR}/plot/output/test_tfmalloc.txt)
        endif()
        
        set(output_tfmalloc_path ${PROJECT_SOURCE_DIR}/plot/output/test_tfmalloc.txt)

        add_custom_command(
            TARGET allocator_tester_tfmalloc
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E env $<TARGET_FILE:allocator_tester_tfmalloc> > "${output_tfmalloc_path}"
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Redirecting output to ${output_tfmalloc_path}"
        )

    endif()

    set(ALLOC_TEST_TFMALLOC FALSE)

endif() # test of tfmalloc

#------------------------------------------------------------------------------
#------------------------------------------------------------------------------
# For new/delete allocators
#------------------------------------------------------------------------------
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# cmake module path for find_package
#------------------------------------------------------------------------------
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

#------------------------------------------------------------------------------
# jemalloc
#------------------------------------------------------------------------------
if(TEST_ALL OR TEST_JEMALLOC)

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

        if (NEW_DELETE_WITH_FANCY_POINTERS)
            target_compile_definitions(allocator_tester_jemalloc PUBLIC NEW_DELETE_WITH_FANCY_POINTERS)
        endif()

        target_link_libraries(allocator_tester_jemalloc PUBLIC ${JEMALLOC_LIBRARY})
    else()
        message(STATUS "Jemalloc was not found")
    return()
    endif()

    if((PRINT_IN_TXT_ALL OR PRINT_IN_TXT_JEMALLOC) AND (TEST_ALL OR TEST_JEMALLOC))

        if(NEW_DELETE_WITH_FANCY_POINTERS)

            if(EXISTS ${PROJECT_SOURCE_DIR}/plot/output/test_fancy_jemalloc.txt)
                message("Removing previous jemalloc result file...")
                file(REMOVE ${PROJECT_SOURCE_DIR}/plot/output/test_fancy_jemalloc.txt)
            endif()
            set(output_jemalloc_path ${PROJECT_SOURCE_DIR}/plot/output/test_fancy_jemalloc.txt)

        else()

            if(EXISTS ${PROJECT_SOURCE_DIR}/plot/output/test_jemalloc.txt)
            message("Removing previous jemalloc result file...")
            file(REMOVE ${PROJECT_SOURCE_DIR}/plot/output/test_jemalloc.txt)
            endif()
            set(output_jemalloc_path ${PROJECT_SOURCE_DIR}/plot/output/test_jemalloc.txt)

        endif()

        add_custom_command(
            TARGET allocator_tester_jemalloc
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E env $<TARGET_FILE:allocator_tester_jemalloc> > "${output_jemalloc_path}"
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Redirecting output to ${output_jemalloc_path}"
        )

    endif()

endif() # test of jemalloc

#------------------------------------------------------------------------------
# mimalloc
#------------------------------------------------------------------------------
if(TEST_ALL OR TEST_MIMALLOC)

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

        if (NEW_DELETE_WITH_FANCY_POINTERS)
            target_compile_definitions(allocator_tester_jemalloc PUBLIC NEW_DELETE_WITH_FANCY_POINTERS)
        endif()
        
        target_link_libraries(allocator_tester_mimalloc PUBLIC mimalloc)
    else()
        message(STATUS "mimalloc was not found")
    return()
    endif()

    if((PRINT_IN_TXT_ALL OR PRINT_IN_TXT_MIMALLOC) AND (TEST_ALL OR TEST_MIMALLOC))

        if(NEW_DELETE_WITH_FANCY_POINTERS)

            if(EXISTS ${PROJECT_SOURCE_DIR}/plot/output/test_fancy_mimalloc.txt)
                message("Removing previous mimalloc result file...")
                file(REMOVE ${PROJECT_SOURCE_DIR}/plot/output/test_fancy_mimalloc.txt)
            endif()
            set(output_mimalloc_path ${PROJECT_SOURCE_DIR}/plot/output/test_fancy_mimalloc.txt)

        else()

            if(EXISTS ${PROJECT_SOURCE_DIR}/plot/output/test_mimalloc.txt)
                message("Removing previous mimalloc result file...")
                file(REMOVE ${PROJECT_SOURCE_DIR}/plot/output/test_mimalloc.txt)
            endif()
            set(output_mimalloc_path ${PROJECT_SOURCE_DIR}/plot/output/test_mimalloc.txt)
            
        endif()

        add_custom_command(
            TARGET allocator_tester_mimalloc
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E env $<TARGET_FILE:allocator_tester_mimalloc> > "${output_mimalloc_path}"
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Redirecting output to ${output_mimalloc_path}"
        )

    endif()

endif() # test of mimalloc

#------------------------------------------------------------------------------
# tcmalloc
#------------------------------------------------------------------------------
if(TEST_ALL OR TEST_TCMALLOC)

    find_package(Tcmalloc QUIET)

    if(Tcmalloc_FOUND)
        message(STATUS "tcmalloc found")

        # Add the executable target
        add_executable(allocator_tester_tcmalloc
        src/test_common.cpp
        src/allocator_tester.cpp
        )
        target_include_directories(allocator_tester_tcmalloc PUBLIC src)

        if (NEW_DELETE_WITH_FANCY_POINTERS)
            target_compile_definitions(allocator_tester_jemalloc PUBLIC NEW_DELETE_WITH_FANCY_POINTERS)
        endif()
        
        target_link_libraries(allocator_tester_tcmalloc PUBLIC ${Tcmalloc_LIBRARY})
    else()
        message(STATUS "tcmalloc was not found")
    return()
    endif()

    if((PRINT_IN_TXT_ALL OR PRINT_IN_TXT_TCMALLOC) AND (TEST_ALL OR TEST_TCMALLOC))

        if(NEW_DELETE_WITH_FANCY_POINTERS)

            if(EXISTS ${PROJECT_SOURCE_DIR}/plot/output/test_fancy_tcmalloc.txt)
                message("Removing previous tcmalloc result file...")
                file(REMOVE ${PROJECT_SOURCE_DIR}/plot/output/test_fancy_tcmalloc.txt)
            endif()
            set(output_tcmalloc_path ${PROJECT_SOURCE_DIR}/plot/output/test_fancy_tcmalloc.txt)

        else()

            if(EXISTS ${PROJECT_SOURCE_DIR}/plot/output/test_tcmalloc.txt)
                message("Removing previous tcmalloc result file...")
                file(REMOVE ${PROJECT_SOURCE_DIR}/plot/output/test_tcmalloc.txt)
            endif()
            set(output_tcmalloc_path ${PROJECT_SOURCE_DIR}/plot/output/test_tcmalloc.txt)
            
        endif()

        add_custom_command(
            TARGET allocator_tester_tcmalloc
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E env $<TARGET_FILE:allocator_tester_tcmalloc> > "${output_tcmalloc_path}"
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Redirecting output to ${output_tcmalloc_path}"
        )

    endif()

endif() # test of tcmalloc