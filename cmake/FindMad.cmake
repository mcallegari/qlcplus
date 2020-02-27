# - Try to find Mad
# Once done this will define
#
#  MAD_FOUND - system has Mad
#  MAD_INCLUDE_DIR - the Mad include directory
#  MAD_LIBRARIES - Link these to use Mad
#  MAD_DEFINITIONS - Compiler switches required for using Mad
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

# Copyright (c) 2007, Laurent Montel, <montel@kde.org>
#


if ( MAD_INCLUDE_DIR AND MAD_LIBRARIES )
   # in cache already
   SET(Mad_FIND_QUIETLY TRUE)
endif ( MAD_INCLUDE_DIR AND MAD_LIBRARIES )

FIND_PATH(MAD_INCLUDE_DIR NAMES mad.h
)

FIND_LIBRARY(MAD_LIBRARIES NAMES mad
)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Mad DEFAULT_MSG MAD_INCLUDE_DIR MAD_LIBRARIES )

# show the MAD_INCLUDE_DIR and MAD_LIBRARIES variables only in the advanced view
MARK_AS_ADVANCED(MAD_INCLUDE_DIR MAD_LIBRARIES )

