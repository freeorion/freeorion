##########################################################################
# Boost Utilities                                                        #
##########################################################################
# Copyright (C) 2007 Douglas Gregor <doug.gregor@gmail.com>              #
# Copyright (C) 2007 Troy Straszheim                                     #
#                                                                        #
# Distributed under the Boost Software License, Version 1.0.             #
# See accompanying file LICENSE_1_0.txt or copy at                       #
#   http://www.boost.org/LICENSE_1_0.txt                                 #
##########################################################################

# This utility macro determines whether a particular string value
# occurs within a list of strings:
#
#  list_contains(result string_to_find arg1 arg2 arg3 ... argn)
# 
# This macro sets the variable named by result equal to TRUE if
# string_to_find is found anywhere in the following arguments.
macro (list_contains var value)
  set(${var})
  foreach (value2 ${ARGN})
    if (${value} STREQUAL ${value2})
      set(${var} TRUE)
    endif (${value} STREQUAL ${value2})
  endforeach (value2)
endmacro ()

macro (get_pkg_config_libs name)
    set(${name})
    get_directory_property(dirs LINK_DIRECTORIES)
    list(APPEND dirs /usr/local/lib /usr/lib /lib)
    set(skip_lib false)
    foreach (lib ${ARGN})
        if (skip_lib)
            set(skip_lib false)
        elseif (lib STREQUAL "optimized" AND NOT CMAKE_BUILD_TYPE STREQUAL "Release")
            set(skip_lib true)
        elseif (lib STREQUAL "debug" AND NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
            set(skip_lib true)
        else ()
            foreach (dir ${dirs})
                string(REPLACE ${dir}/ "" stripped_lib ${lib})
                if (NOT stripped_lib STREQUAL lib)
                    string(
                        REGEX REPLACE
                        "lib(.*)(${CMAKE_SHARED_LIBRARY_SUFFIX}|${CMAKE_STATIC_LIBRARY_SUFFIX})"
                        "${CMAKE_LINK_LIBRARY_FLAG}\\1"
                        stripped_lib
                        ${stripped_lib}
                    )
                    list(APPEND ${name} ${stripped_lib})
                    break()
                endif ()
            endforeach ()
        endif ()
    endforeach ()
    string(REPLACE ";" " " ${name} "${${name}}")
endmacro ()
