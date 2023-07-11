# - Find Tcmalloc
# Find the native Tcmalloc includes and library
#
#  Tcmalloc_INCLUDE_DIR - where to find Tcmalloc.h, etc.
#  Tcmalloc_LIBRARIES   - List of libraries when using Tcmalloc.
#  Tcmalloc_FOUND       - True if Tcmalloc found.

find_package(PkgConfig QUIET)
pkg_check_modules(PC_JEMALLOC QUIET jemalloc)

find_path(
  Tcmalloc_INCLUDE_DIR google/tcmalloc.h
  HINTS ${Tcmalloc_ROOT}
        ${TCMALLOC_ROOT}
        ENV
        TCMALLOC_ROOT
        ${PC_TCMALLOC_MINIMAL_INCLUDEDIR}
        ${PC_TCMALLOC_MINIMAL_INCLUDE_DIRS}
        ${PC_TCMALLOC_INCLUDEDIR}
        ${PC_TCMALLOC_INCLUDE_DIRS}
        /usr/include
        /opt/local/include
        /usr/local/include
  PATH_SUFFIXES include
)

find_library(
  Tcmalloc_LIBRARY
  NAMES tcmalloc_minimal tcmalloc libtcmalloc_minimal libtcmalloc
  HINTS ${Tcmalloc_ROOT}
        ${TCMALLOC_ROOT}
        ENV
        TCMALLOC_ROOT
        ${PC_TCMALLOC_MINIMAL_LIBDIR}
        ${PC_TCMALLOC_MINIMAL_LIBRARY_DIRS}
        ${PC_TCMALLOC_LIBDIR}
        ${PC_TCMALLOC_LIBRARY_DIRS}
  PATH_SUFFIXES lib lib64
)

if(Tcmalloc_INCLUDE_DIR AND Tcmalloc_LIBRARY)
  set(Tcmalloc_FOUND TRUE)
  set(TCMALLOC_FOUND TRUE)
  set(Tcmalloc_LIBRARIES ${Tcmalloc_LIBRARY})
else()
  set(Tcmalloc_FOUND FALSE)
  set(Tcmalloc_LIBRARIES)
endif()

if(NOT Tcmalloc_FOUND)
  message(STATUS "Not Found Tcmalloc: ${Tcmalloc_LIBRARY}")
  if(Tcmalloc_FIND_REQUIRED)
    message(STATUS "Looked for Tcmalloc libraries named ${Tcmalloc_NAMES}.")
    message(FATAL_ERROR "Could NOT find Tcmalloc library")
  endif()
endif()

mark_as_advanced(Tcmalloc_LIBRARY Tcmalloc_INCLUDE_DIR)
