#.rst:
# FindFreetype
# ------------
#
# Locate FreeType library
#
# This module defines
#
# ::
#
#   FREETYPE_LIBRARIES, the library to link against
#   FREETYPE_FOUND, if false, do not try to link to FREETYPE
#   FREETYPE_INCLUDE_DIRS, where to find headers.
#   FREETYPE_VERSION, the version of freetype found (since CMake 2.8.8)
#
#
#
# $FREETYPE_DIR is an environment variable that would correspond to the
# ./configure --prefix=$FREETYPE_DIR used in building FREETYPE.

#=============================================================================
# CMake - Cross Platform Makefile Generator
# Copyright 2000-2011 Kitware, Inc., Insight Software Consortium
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# * Redistributions of source code must retain the above copyright
#   notice, this list of conditions and the following disclaimer.
#
# * Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in the
#   documentation and/or other materials provided with the distribution.
#
# * Neither the names of Kitware, Inc., the Insight Software Consortium,
#   nor the names of their contributors may be used to endorse or promote
#   products derived from this software without specific prior written
#   permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# ------------------------------------------------------------------------------
#
# The above copyright and license notice applies to distributions of
# CMake in source and binary form.  Some source files contain additional
# notices of original copyright by their contributors; see each source
# for details.  Third-party software packages supplied with CMake under
# compatible licenses provide their own copyright notices documented in
# corresponding subdirectories.
#
# ------------------------------------------------------------------------------
#
# CMake was initially developed by Kitware with the following sponsorship:
#
#  * National Library of Medicine at the National Institutes of Health
#    as part of the Insight Segmentation and Registration Toolkit (ITK).
#
#  * US National Labs (Los Alamos, Livermore, Sandia) ASC Parallel
#    Visualization Initiative.
#
#  * National Alliance for Medical Image Computing (NAMIC) is funded by the
#    National Institutes of Health through the NIH Roadmap for Medical Research,
#    Grant U54 EB005149.
#
#  * Kitware, Inc.
#=============================================================================

# Created by Eric Wing.
# Modifications by Alexander Neundorf.
# This file has been renamed to "FindFreetype.cmake" instead of the correct
# "FindFreeType.cmake" in order to be compatible with the one from KDE4, Alex.

# Ugh, FreeType seems to use some #include trickery which
# makes this harder than it should be. It looks like they
# put ft2build.h in a common/easier-to-find location which
# then contains a #include to a more specific header in a
# more specific location (#include <freetype/config/ftheader.h>).
# Then from there, they need to set a bunch of #define's
# so you can do something like:
# #include FT_FREETYPE_H
# Unfortunately, using CMake's mechanisms like include_directories()
# wants explicit full paths and this trickery doesn't work too well.
# I'm going to attempt to cut out the middleman and hope
# everything still works.
find_path(
  FREETYPE_INCLUDE_DIR
    ft2build.h
  HINTS
    ENV FREETYPE_DIR
  PATHS
    ENV GTKMM_BASEPATH
    [HKEY_CURRENT_USER\\SOFTWARE\\gtkmm\\2.4;Path]
    [HKEY_LOCAL_MACHINE\\SOFTWARE\\gtkmm\\2.4;Path]
  PATH_SUFFIXES
    freetype2
    freetype
)

if(EXISTS "${FREETYPE_INCLUDE_DIR}")
    file(STRINGS "${FREETYPE_INCLUDE_DIR}/ft2build.h" _FREETYPE_INCLUDE_ftheader_h
        REGEX "^#[\t ]*include[\t ]+<\(.*/ftheader\\.h\)>[\t ]*$")
    string(REGEX MATCHALL "^#[\t ]*include[\t ]+<\(.*ftheader\\.h\)>[\t ]*$"
           _FREETYPE_INCLUDE_ftheader_h "${_FREETYPE_INCLUDE_ftheader_h}")
    set(_FREETYPE_INCLUDE_ftheader_h "${FREETYPE_INCLUDE_DIR}/${CMAKE_MATCH_1}")
endif()

if(EXISTS "${_FREETYPE_INCLUDE_ftheader_h}")
    file(STRINGS "${_FREETYPE_INCLUDE_ftheader_h}" _FREETYPE_INCLUDE_freetype_h
        REGEX "^#[\t ]*define[\t ]+FT_FREETYPE_H[\t ]+<\(.*\)>[\t ]*$")
    string(REGEX MATCHALL "^#[\t ]*define[\t ]+FT_FREETYPE_H[\t ]+<\(.*\)>[\t ]*$"
           _FREETYPE_INCLUDE_freetype_h "${_FREETYPE_INCLUDE_freetype_h}")
    set(_FREETYPE_INCLUDE_freetype_h "${FREETYPE_INCLUDE_DIR}/${CMAKE_MATCH_1}")
endif()

if(EXISTS "${_FREETYPE_INCLUDE_freetype_h}")
  file(STRINGS "${_FREETYPE_INCLUDE_freetype_h}" freetype_version_str
       REGEX "^#[\t ]*define[\t ]+FREETYPE_(MAJOR|MINOR|PATCH)[\t ]+[0-9]+$")

  unset(FREETYPE_VERSION)
  unset(FREETYPE_FILE_VERSION)
  foreach(VPART MAJOR MINOR PATCH)
    foreach(VLINE ${freetype_version_str})
      if(VLINE MATCHES "^#[\t ]*define[\t ]+FREETYPE_${VPART}[\t ]+([0-9]+)$")
        set(FREETYPE_VERSION_PART "${CMAKE_MATCH_1}")
        if(FREETYPE_VERSION)
          set(FREETYPE_VERSION "${FREETYPE_VERSION}.${FREETYPE_VERSION_PART}")
        else()
          set(FREETYPE_VERSION "${FREETYPE_VERSION_PART}")
        endif()
        set(FREETYPE_FILE_VERSION "${FREETYPE_FILE_VERSION}${FREETYPE_VERSION_PART}")
        unset(FREETYPE_VERSION_PART)
      endif()
    endforeach()
  endforeach()

endif()

find_library(FREETYPE_LIBRARY
  NAMES freetype libfreetype "freetype${FREETYPE_FILE_VERSION}" "freetype${FREETYPE_FILE_VERSION}MT"
  HINTS
    ENV FREETYPE_DIR
  PATHS
    ENV GTKMM_BASEPATH
    [HKEY_CURRENT_USER\\SOFTWARE\\gtkmm\\2.4;Path]
    [HKEY_LOCAL_MACHINE\\SOFTWARE\\gtkmm\\2.4;Path]
)

unset(FREETYPE_FILE_VERSION_STRING)

set(FREETYPE_INCLUDE_DIRS "${FREETYPE_INCLUDE_DIR}")
set(FREETYPE_LIBRARIES "${FREETYPE_LIBRARY}")

# handle the QUIETLY and REQUIRED arguments and set FREETYPE_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
  Freetype
  REQUIRED_VARS
    FREETYPE_LIBRARIES
    FREETYPE_INCLUDE_DIRS
  VERSION_VAR
    FREETYPE_VERSION
)

mark_as_advanced(
  FREETYPE_LIBRARIES
  FREETYPE_INCLUDE_DIRS
  FREETYPE_VERSION
)
