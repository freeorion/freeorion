# GG is a GUI for SDL and OpenGL.
#
# Copyright (C) 2016 Marcel Metz
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public License
# as published by the Free Software Foundation; either version 2.1
# of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free
# Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
# 02111-1307 USA
#
# If you do not wish to comply with the terms of the LGPL please
# contact the author as other terms are available for a fee.
#
# Zach Laine
# whatwasthataddress@gmail.com

set(CMAKE_C_FLAGS_COVERAGE "-g -O0 -fprofile-arcs -ftest-coverage")
set(CMAKE_CXX_FLAGS_COVERAGE "-g -O0 -fprofile-arcs -ftest-coverage")
set(CMAKE_EXE_LINKER_FLAGS_COVERAGE "-fprofile-arcs -ftest-coverage")
set(CMAKE_SHARED_LINKER_FLAGS_COVERAGE "-fprofile-arcs -ftest-coverage")
set(CMAKE_ARCHIVE_LINKER_FLAGS_COVERAGE "-fprofile-arcs -ftest-coverage")

mark_as_advanced(
    CMAKE_C_FLAGS_COVERAGE
    CMAKE_CXX_FLAGS_COVERAGE
    CMAKE_EXE_LINKER_FLAGS_COVERAGE
    CMAKE_SHARED_LINKER_FLAGS_COVERAGE
    CMAKE_ARCHIVE_LINKER_FLAGS_COVERAGE
)

set(CMAKE_BUILD_TYPE "${CMAKE_BUILD_TYPE}" CACHE STRING
    "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel Coverage")

if(CMAKE_CONFIGURATION_TYPES)
    list(APPEND CMAKE_CONFIGURATION_TYPES Coverage)
    list(REMOVE_DUPLICATES CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_CONFIGURATION_TYPES "${CMAKE_CONFIGURATION_TYPES}" CACHE STRING
        "Available types of build"
        FORCE)
endif()

set(COVERAGE_REPORT_TYPE "HTML" CACHE STRING "The coverage report format (one of: `HTML`, `XML`)")

function(ENABLE_COVERAGE)
    if(TARGET coverage)
        return()
    endif()

    if(NOT CMAKE_BUILD_TYPE STREQUAL "Coverage")
        message(WARNING "Code coverage requires \"Coverage\" build configuration. Disabling code coverage.")
        return()
    endif()

    find_program(GCOV_EXECUTABLE gcov)
    find_program(GCOVR_EXECUTABLE gcovr)

    if(NOT GCOV_EXECUTABLE OR NOT GCOVR_EXECUTABLE)
        message(WARNING "Could not find `gcov` and `gcovr`. Disabling code coverage.")
        return()
    endif()

    if("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
        if("${CMAKE_CXX_COMPILER_VERSION}" VERSION_LESS 3)
            message(FATAL_ERROR "Requires Clang 3.0.0 or greater to create code coverage")
        endif()
    elseif(NOT "${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
        message(FATAL_ERROR "Requires GNU GCC or Clang to create code coverage")
    endif()

    if("${COVERAGE_REPORT_TYPE}" STREQUAL "XML")
        set(COVERAGE_REPORT_TYPE_FLAGS
            --xml-pretty
            --output ${CMAKE_PROJECT_NAME}.cov.xml
        )
    else()
        set(COVERAGE_REPORT_AUX_COMMANDS
            COMMAND
                ${CMAKE_COMMAND} -E remove_directory coverage
            COMMAND
                ${CMAKE_COMMAND} -E make_directory coverage
        )
        set(COVERAGE_REPORT_TYPE_FLAGS
            --html
            --html-details
            --output coverage/${CMAKE_PROJECT_NAME}.html
        )
    endif()

    add_custom_target(coverage-cpp
        ${COVERAGE_REPORT_AUX_COMMANDS}
        # Capture gcov line counters
        COMMAND
            ${GCOVR_EXECUTABLE}
                --gcov-executable ${GCOV_EXECUTABLE}
                --object-directory ${CMAKE_BINARY_DIR}
                --root ${CMAKE_SOURCE_DIR}
                --delete
                ${COVERAGE_REPORT_TYPE_FLAGS}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Generate C++ code coverage report"
    )

    add_custom_target(coverage
        COMMENT "Create code coverage for ${CMAKE_PROJECT_NAME}")

    add_dependencies(coverage coverage-cpp)
endfunction()

function(ADD_COVERAGE _TARGET _TEST_TARGET)
    if(NOT TARGET coverage)
        return()
    endif()

    add_dependencies(coverage-cpp ${_TARGET})
    add_dependencies(coverage-cpp ${_TEST_TARGET})
endfunction()
