# - Find Bullet includes and library
#
# This module defines
#  BULLET_INCLUDE_DIR
#  BULLET_LIBRARIES, the libraries to link against to use Bullet.
#  BULLET_LIB_DIR, the location of the libraries
#  BULLET_FOUND, If false, do not try to use Bullet
#
# Copyright © 2007, Matt Williams
# Changes for Bullet detection by Garvek, 2009
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
 
IF (BULLET_LIBRARIES AND BULLET_INCLUDE_DIR)
    SET(BULLET_FIND_QUIETLY TRUE) # Already in cache, be silent
ENDIF (BULLET_LIBRARIES AND BULLET_INCLUDE_DIR)
 
IF (WIN32) #Windows
    MESSAGE(STATUS "Looking for Bullet")
    SET(BULLET_SDK $ENV{BULLET_HOME})
    IF (BULLET_SDK)
        MESSAGE(STATUS "Using Bullet SDK")
        STRING(REGEX REPLACE "[\\]" "/" BULLET_SDK "${BULLET_SDK}")
        SET(BULLET_INCLUDE_DIR ${BULLET_SDK}/src)
        SET(BULLET_LIB_DIR ${BULLET_SDK}/lib)
        SET(BULLET_LIBRARIES debug BulletCollision BulletDynamics LinearMath
            optimized BulletCollision BulletDynamics LinearMath)
    ENDIF (BULLET_SDK)
ELSE (WIN32) #Unix
    # No .pc for this one
    SET(BULLET_SDK $ENV{BULLET_HOME})
    FIND_PATH(BULLET_INCLUDE_DIR btBulletDynamicsCommon.h PATHS /usr/local/include /usr/include ${BULLET_SDK} PATH_SUFFIXES src bullet)

    FIND_LIBRARY(LIB_DYNAMICS bulletdynamics BulletDynamics PATHS ${BULLET_SDK}/lib /usr/lib /usr/local/lib)
    FIND_LIBRARY(LIB_COLLISION bulletcollision BulletCollision PATHS ${BULLET_SDK}/lib /usr/lib /usr/local/lib)
    FIND_LIBRARY(LIB_LINEARMATH bulletmath LinearMath PATHS ${BULLET_SDK}/lib /usr/lib /usr/local/lib)

    IF (BULLET_INCLUDE_DIR AND LIB_DYNAMICS AND LIB_COLLISION AND LIB_LINEARMATH)
        # Must link in this order!
        SET(BULLET_LIBRARIES ${LIB_DYNAMICS} ${LIB_COLLISION} ${LIB_LINEARMATH})
        SET(BULLET_FOUND "YES")
    ELSE (BULLET_INCLUDE_DIR AND LIB_DYNAMICS AND LIB_COLLISION AND LIB_LINEARMATH)
        SET(BULLET_FOUND "NO")
    ENDIF (BULLET_INCLUDE_DIR AND LIB_DYNAMICS AND LIB_COLLISION AND LIB_LINEARMATH)
ENDIF (WIN32)
 
#Do some preparation
SEPARATE_ARGUMENTS(BULLET_INCLUDE_DIR)
SEPARATE_ARGUMENTS(BULLET_LIBRARIES)
 
SET(BULLET_INCLUDE_DIR ${BULLET_INCLUDE_DIR} CACHE PATH "")
SET(BULLET_LIBRARIES ${BULLET_LIBRARIES} CACHE STRING "")
SET(BULLET_LIB_DIR ${BULLET_LIB_DIR} CACHE PATH "")
 
IF (BULLET_INCLUDE_DIR AND BULLET_LIBRARIES)
    SET(BULLET_FOUND TRUE)
ENDIF (BULLET_INCLUDE_DIR AND BULLET_LIBRARIES)
 
IF (BULLET_FOUND)
    IF (NOT BULLET_FIND_QUIETLY)
        MESSAGE(STATUS "  libraries : ${BULLET_LIBRARIES} from ${BULLET_LIB_DIR}")
        MESSAGE(STATUS "  includes  : ${BULLET_INCLUDE_DIR}")
    ENDIF (NOT BULLET_FIND_QUIETLY)
ELSE (BULLET_FOUND)
    IF (BULLET_FIND_REQUIRED)
        MESSAGE(FATAL_ERROR "Could not find Bullet")
    ENDIF (BULLET_FIND_REQUIRED)
ENDIF (BULLET_FOUND)
 
