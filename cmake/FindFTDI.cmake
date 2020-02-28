# - Try to find FTDI
# Once done this will define
#
#  FTDI_FOUND - system has FTDI
#  FTDI_INCLUDE_DIRS - the FTDI include directory
#  FTDI_LIBRARIES - Link these to use FTDI
#  FTDI_DEFINITIONS - Compiler switches required for using FTDI
#  FTDI_VERSION - FTDI version
#
#  Copyright (c) 2006 Andreas Schneider <mail@cynapses.org>
#
# Redistribution and use is allowed according to the terms of the New BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#


if (FTDI_LIBRARIES AND FTDI_INCLUDE_DIRS)
  # in cache already
  set(FTDI_FOUND TRUE)
else (FTDI_LIBRARIES AND FTDI_INCLUDE_DIRS)
  if (NOT WIN32)
   include(FindPkgConfig)
   pkg_check_modules(FTDI libftdi1)
  endif (NOT WIN32)

  if (FTDI_FOUND)
    set(FTDI_INCLUDE_DIRS
      ${FTDI_INCLUDE_DIRS}
    )
    if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
      set(FTDI_LIBRARIES "${FTDI_LIBRARY_DIRS}/lib${FTDI_LIBRARIES}.dylib")
    else (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
      set(FTDI_LIBRARIES
        ${FTDI_LIBRARIES}
      )
    endif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(FTDI_VERSION
      1.2
    )
    set(FTDI_FOUND TRUE)
  else (FTDI_FOUND)
    find_path(FTDI_INCLUDE_DIR
      NAMES
        ftdi.h
      PATHS
        /usr/include
        /usr/local/include
        /opt/local/include
        /sw/include
    )
   
    find_library(FTDI_LIBRARY
      NAMES
        ftdi
      PATHS
        /usr/lib
        /usr/local/lib
        /opt/local/lib
        /sw/lib
        ${FTDI_LIBRARY_DIR}
    )
   
    set(FTDI_INCLUDE_DIRS
      ${FTDI_INCLUDE_DIR}
    )
    set(FTDI_LIBRARIES
      ${FTDI_LIBRARY}
    )
   
    set(FTDI_LIBRARY_DIRS
      ${FTDI_LIBRARY_DIR}
    )
   
    set(FTDI_VERSION
      1.2
    )
   
    if (FTDI_INCLUDE_DIRS AND FTDI_LIBRARIES)
       set(FTDI_FOUND TRUE)
    endif (FTDI_INCLUDE_DIRS AND FTDI_LIBRARIES)
   
    if (FTDI_FOUND)
      if (NOT FTDI_FIND_QUIETLY)
        message(STATUS "Found FTDI: ${FTDI_LIBRARIES}")
      endif (NOT FTDI_FIND_QUIETLY)
    else (FTDI_FOUND)
      if (FTDI_FIND_REQUIRED)
        message(FATAL_ERROR "Could not find FTDI")
      endif (FTDI_FIND_REQUIRED)
    endif (FTDI_FOUND)
  endif (FTDI_FOUND)


  # show the FTDI_INCLUDE_DIRS and FTDI_LIBRARIES variables only in the advanced view
  mark_as_advanced(FTDI_INCLUDE_DIRS FTDI_LIBRARIES)

endif (FTDI_LIBRARIES AND FTDI_INCLUDE_DIRS)
