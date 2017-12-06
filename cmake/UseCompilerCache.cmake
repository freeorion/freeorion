# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

# .rst:
# UseCompilerCache
# --------
#
# This module provides a function to setup a compiler cache tool (currently ``ccache``):
#
#   USE_COMPILER_CACHE([PROGRAM <ccache_name>] [QUIET] [REQUIRED])
#   -- Add the compiler cache tool (default to look for ccache on the path)
#      to your build through CMAKE_<LANG>_COMPILER_LAUNCHER variables. Also
#      supports XCode. Uses a wrapper for XCode and CCache < 3.3.
#      Sets the CCACHE_VERSION variable.

# TODO: Remove this module when cmake 3.11 is the minimum required version
#       FreeOrion included this file directly from CMake master.  

function(USE_COMPILER_CACHE)
    set(options
        QUIET
        REQUIRED
    )

    set(oneValueArgs
        PROGRAM
    )

    set(multiValueArgs
    )

    cmake_parse_arguments(ARGS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT ARGS_PROGRAM)
        set(ARGS_PROGRAM ccache)
    endif()

    find_program(CCACHE_PROGRAM ${ARGS_PROGRAM})

    # Quit if required and not found
    if(REQUIRED AND NOT CCACHE_PROGRAM)
        message(FATAL_ERROR "Failed to find ${CCACHE_PROGRAM} (REQUIRED)")
    endif()

    # Only add if program found
    if(CCACHE_PROGRAM)
        # Get version number
        execute_process(COMMAND "${CCACHE_PROGRAM}" --version OUTPUT_VARIABLE output)
        string(REPLACE "\n" ";" output "${output}")
        foreach(line ${output})
            string(TOLOWER ${line} line)
            string(REGEX REPLACE "ccache version ([\\.0-9]+)$" "\\1" version "${line}")
            if(version AND NOT "x${line}" STREQUAL "x${version}")
                set(COMPILER_CACHE_VERSION ${version})
                break()
            endif()
        endforeach()

        if(NOT QUIET)
            message(STATUS "CCache version: ${COMPILER_CACHE_VERSION}")
        endif()

        # TODO: set PARENT_SCOPE with COMPILER_CACHE_VERSION

        # This wrapper only is needed for CCache < 3.3 or XCode
        if("${CMAKE_GENERATOR}" STREQUAL Xcode OR "${CCACHE_VERSION}" VERSION_LESS 3.3.0)
            file(WRITE "${CMAKE_BINARY_DIR}/launch-c" ""
                "#!/bin/sh\n"
                "\n"
                "# Xcode generator doesn't include the compiler as the\n"
                "# first argument, Ninja and Makefiles do. Handle both cases.\n"
                "if [[ \"$1\" = \"${CMAKE_C_COMPILER}\" ]] ; then\n"
                "    shift\n"
                "fi\n"
                "\n"
                "export CCACHE_CPP2=true\n"
                "exec \"${CCACHE_PROGRAM}\" \"${CMAKE_C_COMPILER}\" \"$@\"\n"
                )

            file(WRITE "${CMAKE_BINARY_DIR}/launch-cxx" ""
                "#!/bin/sh\n"
                "\n"
                "# Xcode generator doesn't include the compiler as the\n"
                "# first argument, Ninja and Makefiles do. Handle both cases.\n"
                "if [[ \"$1\" = \"${CMAKE_CXX_COMPILER}\" ]] ; then\n"
                "    shift\n"
                "fi\n"
                "\n"
                "export CCACHE_CPP2=true\n"
                "exec \"${CCACHE_PROGRAM}\" \"${CMAKE_CXX_COMPILER}\" \"$@\"\n"
                )

            # Cuda support only added in CMake 3.10
            file(WRITE "${CMAKE_BINARY_DIR}/launch-cuda" ""
                "#!/bin/sh\n"
                "\n"
                "# Xcode generator doesn't include the compiler as the\n"
                "# first argument, Ninja and Makefiles do. Handle both cases.\n"
                "if [[ \"$1\" = \"${CMAKE_CUDA_COMPILER}\" ]] ; then\n"
                "    shift\n"
                "fi\n"
                "\n"
                "export CCACHE_CPP2=true\n"
                "exec \"${CCACHE_PROGRAM}\" \"${CMAKE_CUDA_COMPILER}\" \"$@\"\n"
                )


            execute_process(COMMAND chmod a+rx
                             "${CMAKE_BINARY_DIR}/launch-c"
                             "${CMAKE_BINARY_DIR}/launch-cxx"
                             "${CMAKE_BINARY_DIR}/launch-cuda"
            )

            # Set Xcode project attributes to route compilation and linking
            # through our scripts
            set(CMAKE_XCODE_ATTRIBUTE_CC         "${CMAKE_BINARY_DIR}/launch-c")
            set(CMAKE_XCODE_ATTRIBUTE_CXX        "${CMAKE_BINARY_DIR}/launch-cxx")
            set(CMAKE_XCODE_ATTRIBUTE_LD         "${CMAKE_BINARY_DIR}/launch-c")
            set(CMAKE_XCODE_ATTRIBUTE_LDPLUSPLUS "${CMAKE_BINARY_DIR}/launch-cxx")

        else()
            # Support Unix Makefiles and Ninja
            set(CMAKE_C_COMPILER_LAUNCHER   "${CCACHE_PROGRAM}")
            set(CMAKE_CXX_COMPILER_LAUNCHER "${CMAKE_PROGRAM}")
            set(CMAKE_CUDA_COMPILER_LAUNCHER "${CMAKE_PROGRAM}")
        endif()
    endif()
endfunction()
