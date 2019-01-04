# Find QtNodes
# ~~~~~~~~
# Copyright (c) 2018, Frank Stettner <frank-stettner at gmx.net>
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#
# Once run this will define:
#
# QTNODES_FOUND       = system has QtNodes lib
# QTNODES_LIBRARY     = full path to the QtNodes library
# QTNODES_INCLUDE_DIR = where to find headers
#


set(QTNODES_LIBRARY_NAMES qtnodes nodes libnodes)

find_library(QTNODES_LIBRARY
  NAMES ${QTNODES_LIBRARY_NAMES}
  PATHS
    /usr/lib
    /usr/local/lib
    "$ENV{LIB_DIR}/lib"
    "$ENV{LIB}"
)

FIND_PATH(QTNODES_INCLUDE_DIR
  NAMES NodeDataModel
  PATHS
    /usr/include
    /usr/local/include
    "$ENV{LIB_DIR}/include"
    "$ENV{INCLUDE}"
  PATH_SUFFIXES
    nodes
    libnodes
)

IF (QTNODES_INCLUDE_DIR AND QTNODES_LIBRARY)
  SET(QTNODES_FOUND TRUE)
ENDIF (QTNODES_INCLUDE_DIR AND QTNODES_LIBRARY)

IF (QTNODES_FOUND)
  IF (NOT QTNODES_FIND_QUIETLY)
    MESSAGE(STATUS "Found QtNodes: ${QTNODES_LIBRARY}")
  ENDIF (NOT QTNODES_FIND_QUIETLY)
ELSE (QTNODES_FOUND)
  IF (QTNODES_FIND_QUIETLY)
    MESSAGE(FATAL_ERROR "Could not find QtNodes")
  ENDIF (QTNODES_FIND_QUIETLY)
ENDIF (QTNODES_FOUND)
