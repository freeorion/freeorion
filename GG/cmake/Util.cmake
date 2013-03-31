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

function (pretty_print list)
    if (DEFINED ${list})
        if ("${${list}}" MATCHES ";")
            string(REPLACE ";" "\n  " pretty_list "${${list}}")
            message("${list}=\n  ${pretty_list}")
        else ()
            message("${list}= ${${list}}")
        endif ()
    else ()
        message("${list}= (undefined)")
    endif ()
endfunction ()

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

# This macro is an internal utility macro that builds the name of a
# particular variant of a library
#
#   library_variant_display_name(feature1 feature2 ...)
#
# where feature1, feature2, etc. are the names of features to be
# included in this variant, e.g., MULTI_THREADED, DEBUG. 
#
# This macro sets the string VARIANT_DISPLAY_NAME.. This is the display name
# that describes this variant, e.g., "Debug, static, multi-threaded".
#
macro (library_variant_display_name)
  # Add -static for static libraries, -shared for shared libraries
  list_contains(VARIANT_IS_STATIC STATIC ${ARGN})
  if (VARIANT_IS_STATIC)
    set(VARIANT_DISPLAY_NAME "Static")
  else (VARIANT_IS_STATIC)
    set(VARIANT_DISPLAY_NAME "Shared")
  endif (VARIANT_IS_STATIC)

  # Add "multi-threaded" to the display name for multithreaded libraries.
  list_contains(VARIANT_IS_MT MULTI_THREADED ${ARGN})
  if (VARIANT_IS_MT)
    set(VARIANT_DISPLAY_NAME "${VARIANT_DISPLAY_NAME}, multi-threaded")
  endif ()

  # Add "debug" or "relase" to the display name.
  list_contains(VARIANT_IS_DEBUG DEBUG ${ARGN})
  if (VARIANT_IS_DEBUG)
    set(VARIANT_DISPLAY_NAME "${VARIANT_DISPLAY_NAME}, debug")
  else ()
    set(VARIANT_DISPLAY_NAME "${VARIANT_DISPLAY_NAME}, release")
  endif ()
endmacro (library_variant_display_name)

# This macro is an internal utility macro that updates compilation and
# linking flags based on interactions among the features in a variant.
#
#   feature_interactions(prefix
#                        feature1 feature2 ...)
#
# where "prefix" is the prefix of the compilation and linking flags
# that will be updated (e.g., ${prefix}_COMPILE_FLAGS). feature1,
# feature2, etc. are the names of the features used in this particular
# variant. If the features in this variant conflict, set
# ${prefix}_OKAY to FALSE.
macro (feature_interactions PREFIX)
  # Don't build or link against a shared library and a static run-time
  list_contains(IS_SHARED SHARED ${ARGN})
  list_contains(IS_STATIC STATIC ${ARGN})
  if (IS_SHARED AND IS_STATIC)
    set(${PREFIX}_OKAY FALSE)
  endif (IS_SHARED AND IS_STATIC)

  # Visual C++-specific runtime library flags; with Visual C++, the dynamic
  # runtime is multi-threaded only
  if (MSVC)
    list_contains(IS_SINGLE_THREADED SINGLE_THREADED ${ARGN})
    if (IS_SHARED AND IS_SINGLE_THREADED)
      set(${PREFIX}_OKAY FALSE)
    endif (IS_SHARED AND IS_SINGLE_THREADED) 

    list_contains(IS_DEBUG DEBUG ${ARGN})
    if (IS_DEBUG)
      if (IS_STATIC)
        list(APPEND ${PREFIX}_COMPILE_FLAGS "/MTd")
      else (IS_STATIC)
        list(APPEND ${PREFIX}_COMPILE_FLAGS "/MDd")
      endif (IS_STATIC)       
    else (IS_DEBUG)
      if (IS_STATIC)
        list(APPEND ${PREFIX}_COMPILE_FLAGS "/MT")
      else (IS_STATIC)
        list(APPEND ${PREFIX}_COMPILE_FLAGS "/MD")
      endif (IS_STATIC)       
    endif (IS_DEBUG)
  endif (MSVC)  
endmacro ()

# This macro is an internal utility macro that builds a particular variant of
# a library.
#
#   library_variant(libname 
#                   feature1 feature2 ...)
#
# where libname is the name of the library (e.g., "GiGiSDL") and feature1,
# feature2, ... are the features that will be used in this variant.
#
# This macro will define a new library target based on libname and the
# specific variant name, which depends on the utility target libname. The
# compilation and linking flags for this library are defined by
# THIS_LIB_COMPILE_FLAGS, THIS_LIB_LINK_FLAGS, THIS_LIB_LINK_LIBS, and all of
# the compile and linking flags implied by the features provided.
#
# If any of the features listed conflict with this library, no new targets
# will be built.
macro (library_variant LIBNAME)
  set(THIS_VARIANT_COMPILE_FLAGS "${THIS_LIB_COMPILE_FLAGS}")
  set(THIS_VARIANT_LINK_FLAGS "${THIS_LIB_LINK_FLAGS}")
  set(THIS_VARIANT_LINK_LIBS ${THIS_LIB_LINK_LIBS})

  # Determine if it is okay to build this variant
  set(THIS_VARIANT_OKAY TRUE)
  foreach (ARG ${ARGN})
    # If the user specified that we should not build any variants of
    # this kind, don't. For example, if the BUILD_SHARED option is
    # off, don't build shared libraries.
    if (NOT BUILD_${ARG})
      set(THIS_VARIANT_OKAY FALSE)
    endif (NOT BUILD_${ARG})

    # Accumulate compile and link flags
    list(APPEND THIS_VARIANT_COMPILE_FLAGS ${THIS_LIB_${ARG}_COMPILE_FLAGS} ${${ARG}_COMPILE_FLAGS})
    list(APPEND THIS_VARIANT_LINK_FLAGS ${THIS_LIB_${ARG}_LINK_FLAGS} ${${ARG}_LINK_FLAGS})
    list(APPEND THIS_VARIANT_LINK_LIBS ${THIS_LIB_${ARG}_LINK_LIBS} ${${ARG}_LINK_LIBS})
  endforeach (ARG ${ARGN})

  # Handle feature interactions
  feature_interactions(THIS_VARIANT ${ARGN})

  if (THIS_VARIANT_OKAY)
    library_variant_display_name(${ARGN})

    if (IS_STATIC)
      set(VARIANT_LIBNAME ${LIBNAME}_static)
    else ()
      set(VARIANT_LIBNAME ${LIBNAME})
    endif ()

    # We handle static vs. dynamic libraries differently
    list_contains(THIS_LIB_IS_STATIC STATIC ${ARGN})
    if (THIS_LIB_IS_STATIC)
      # On Windows, we need static and shared libraries to have
      # different names, so we prepend "lib" to the name.
      if (WIN32 AND NOT CYGWIN)
        set(LIBPREFIX "lib")
      else (WIN32 AND NOT CYGWIN)
        set(LIBPREFIX "")
      endif (WIN32 AND NOT CYGWIN)

      # Add the library itself
      add_library(${VARIANT_LIBNAME} STATIC ${THIS_LIB_SOURCES})

      # Set properties on this library
      set_target_properties(${VARIANT_LIBNAME}
        PROPERTIES
        OUTPUT_NAME "${LIBPREFIX}${LIBNAME}"
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
        CLEAN_DIRECT_OUTPUT 1
        COMPILE_FLAGS "${THIS_VARIANT_COMPILE_FLAGS}"
        LINK_FLAGS "${THIS_VARIANT_LINK_FLAGS}"
        LINK_SEARCH_END_STATIC true
        LABELS "${PROJECT_NAME}"
        )
    elseif (THIS_LIB_MODULE)
      # Add a module
      add_library(${VARIANT_LIBNAME} MODULE ${THIS_LIB_SOURCES})

      # Set properties on this library
      set_target_properties(${VARIANT_LIBNAME}
        PROPERTIES
        OUTPUT_NAME ${LIBNAME}
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
        CLEAN_DIRECT_OUTPUT 1
        COMPILE_FLAGS "${THIS_VARIANT_COMPILE_FLAGS}"
        LINK_FLAGS "${THIS_VARIANT_LINK_FLAGS}"
        LABELS "${PROJECT_NAME}"
        PREFIX ""
       # SOVERSION "${BOOST_VERSION}"
        )
    else (THIS_LIB_IS_STATIC)
      #TODO: Check the SOVERSION behavior on Linux and Windows
      # Add a module
      add_library(${VARIANT_LIBNAME} SHARED ${THIS_LIB_SOURCES})
      # Set properties on this library
      set_target_properties(${VARIANT_LIBNAME}
        PROPERTIES
        OUTPUT_NAME ${LIBNAME}
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
        CLEAN_DIRECT_OUTPUT 1
        COMPILE_FLAGS "${THIS_VARIANT_COMPILE_FLAGS}"
        LINK_FLAGS "${THIS_VARIANT_LINK_FLAGS}"
        LABELS "${PROJECT_NAME}"
        # SOVERSION "${BOOST_VERSION}"
        )
    endif (THIS_LIB_IS_STATIC)

    # Link against whatever libraries this library depends on
    target_link_libraries(${VARIANT_LIBNAME} ${THIS_VARIANT_LINK_LIBS})
    foreach (dependency ${THIS_LIB_DEPENDS})
      target_link_libraries(${VARIANT_LIBNAME} "${dependency}")
    endforeach (dependency)

    # Setup installation properties
    string(TOUPPER COMPONENT_${PROJECT_NAME} LIB_COMPONENT)

    # Installation of this library variant
    if ((THIS_LIB_IS_STATIC AND NOT RUNTIME_ONLY_PACKAGE OR
         NOT THIS_LIB_IS_STATIC AND NOT DEVEL_ONLY_PACKAGE) AND
        COMMAND package_compatible)
        package_compatible(ok_to_install ${LIBNAME})
        if (ok_to_install)
            install(
                TARGETS ${VARIANT_LIBNAME}
                RUNTIME DESTINATION bin COMPONENT ${LIB_COMPONENT}
                LIBRARY DESTINATION lib${LIB_SUFFIX} COMPONENT ${LIB_COMPONENT}
                ARCHIVE DESTINATION lib${LIB_SUFFIX} COMPONENT ${LIB_COMPONENT}_DEVEL
            )
        endif ()
    endif ()
  endif ()
endmacro ()

macro (library_all_variants LIBNAME)
    library_variant(${LIBNAME} STATIC DEBUG MULTI_THREADED)
    library_variant(${LIBNAME} STATIC DEBUG SINGLE_THREADED)
    library_variant(${LIBNAME} STATIC RELEASE MULTI_THREADED)
    library_variant(${LIBNAME} STATIC RELEASE SINGLE_THREADED)
    library_variant(${LIBNAME} SHARED DEBUG MULTI_THREADED)
    library_variant(${LIBNAME} SHARED DEBUG SINGLE_THREADED)
    library_variant(${LIBNAME} SHARED RELEASE MULTI_THREADED)
    library_variant(${LIBNAME} SHARED RELEASE SINGLE_THREADED)
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
