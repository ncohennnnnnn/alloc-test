# - Find pmimalloc_none
# Find the native pmimalloc_none includes and library
#
#  pmimalloc_none_INCLUDE_DIR - where to find pmimalloc_none.h, etc.
#  pmimalloc_none_LIBRARIES   - List of libraries when using pmimalloc_none.
#  pmimalloc_none_FOUND       - True if pmimalloc_none found.

find_package(PkgConfig QUIET)
pkg_check_modules(PC_PMIMALLOC_NONE QUIET tcmalloc)

find_path(
  pmimalloc_none_INCLUDE_DIR google/tcmalloc.h
  HINTS ${pmimalloc_none_ROOT}
        ${PMIMALLOC_NONE_ROOT}
        ENV
        PMIMALLOC_NONE_ROOT
        ${PC_PMIMALLOC_NONE_MINIMAL_INCLUDEDIR}
        ${PC_PMIMALLOC_NONE_MINIMAL_INCLUDE_DIRS}
        ${PC_PMIMALLOC_NONE_INCLUDEDIR}
        ${PC_PMIMALLOC_NONE_INCLUDE_DIRS}
        /usr/include
        /opt/local/include
        /usr/local/include
  PATH_SUFFIXES include
)

find_library(
  pmimalloc_none_LIBRARY
  NAMES tcmalloc_minimal tcmalloc libtcmalloc_minimal libtcmalloc
  HINTS ${pmimalloc_none_ROOT}
        ${PMIMALLOC_NONE_ROOT}
        ENV
        PMIMALLOC_NONE_ROOT
        ${PC_PMIMALLOC_NONE_MINIMAL_LIBDIR}
        ${PC_PMIMALLOC_NONE_MINIMAL_LIBRARY_DIRS}
        ${PC_PMIMALLOC_NONE_LIBDIR}
        ${PC_PMIMALLOC_NONE_LIBRARY_DIRS}
  PATH_SUFFIXES lib lib64
)

if(pmimalloc_none_INCLUDE_DIR AND pmimalloc_none_LIBRARY)
  set(pmimalloc_none_FOUND TRUE)
  set(PMIMALLOC_NONE_FOUND TRUE)
  set(pmimalloc_none_LIBRARIES ${pmimalloc_none_LIBRARY})
else()
  set(pmimalloc_none_FOUND FALSE)
  set(pmimalloc_none_LIBRARIES)
endif()

if(NOT pmimalloc_none_FOUND)
  message(STATUS "Not Found pmimalloc_none: ${pmimalloc_none_LIBRARY}")
  if(pmimalloc_none_FIND_REQUIRED)
    message(STATUS "Looked for pmimalloc_none libraries named ${pmimalloc_none_NAMES}.")
    message(FATAL_ERROR "Could NOT find pmimalloc_none library")
  endif()
endif()

mark_as_advanced(pmimalloc_none_LIBRARY pmimalloc_none_INCLUDE_DIR)
