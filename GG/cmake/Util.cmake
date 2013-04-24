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

# This macro is an internal utility macro that builds a particular variant of
# a library.
#
#   library_all_variants(libname)
#
# where libname is the name of the library (e.g., "GiGiSDL")
#
# This macro will define a new library target based on libname and the
# specific variant name, which depends on the utility target libname. The
# compilation and linking flags for this library are defined by
# THIS_LIB_COMPILE_FLAGS, THIS_LIB_LINK_FLAGS, THIS_LIB_LINK_LIBS, and all of
# the compile and linking flags implied by the features provided.
#
# If any of the features listed conflict with this library, no new targets
# will be built.
macro (library_all_variants LIBNAME)
  if (BUILD_SHARED)
    add_library(${LIBNAME} SHARED ${THIS_LIB_SOURCES})
  else ()
    add_library(${LIBNAME} STATIC ${THIS_LIB_SOURCES})
    set_target_properties(${LIBNAME}
      PROPERTIES
      LINK_SEARCH_END_STATIC true
    )
  endif ()

  # Link against whatever libraries this library depends on
  target_link_libraries(${LIBNAME} ${THIS_VARIANT_LINK_LIBS} ${THIS_LIB_LINK_LIBS})

  # Setup installation properties
  string(TOUPPER COMPONENT_${PROJECT_NAME} LIB_COMPONENT)

  # Installation of this library variant
  if ((BUILD_STATIC AND NOT RUNTIME_ONLY_PACKAGE AND COMMAND package_compatible) OR
      (BUILD_SHARED AND NOT DEVEL_ONLY_PACKAGE AND COMMAND package_compatible))
      package_compatible(ok_to_install ${LIBNAME})
      if (ok_to_install)
          install(
              TARGETS ${LIBNAME}
              LIBRARY DESTINATION lib${LIB_SUFFIX} COMPONENT ${LIB_COMPONENT}
              ARCHIVE DESTINATION lib${LIB_SUFFIX} COMPONENT ${LIB_COMPONENT}_DEVEL
          )
      endif ()
  endif ()
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
