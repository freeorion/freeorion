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

function(ENABLE_COVERAGE)
    if(NOT CMAKE_BUILD_TYPE STREQUAL "Coverage")
        message(WARNING "Code coverage inaccurate with an other build configuration than \"Coverage\"")
    endif()

    find_program(GCOV_EXECUTABLE gcov)
    find_program(LCOV_EXECUTABLE lcov)
    find_program(GENHTML_EXECUTABLE genhtml)

    if(NOT GCOV_EXECUTABLE OR NOT LCOV_EXECUTABLE OR NOT GENHTML_EXECUTABLE)
        return()
    endif()

    if("${CMAKE_CXX_COMPILER_ID}" MATCHES "CLANG")
        if("${CMAKE_CXX_COMPILER_VERSION}" VERSION_LESS 3)
            message(FATAL_ERROR "Requires Clang 3.0.0 or greater to create code coverage")
        endif()
    elseif(NOT "${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
        message(FATAL_ERROR "Requires GNU GCC or Clang to create code coverage")
    endif()

    set(COVERAGE_INFO "${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.covinfo")
    set(COVERAGE_CLEANED "${COVERAGE_INFO}.cleaned")

    add_custom_target(${CMAKE_PROJECT_NAME}_coverage_clear
        # Reset line counters
        COMMAND ${LCOV_EXECUTABLE} --quiet --directory . --zerocounters
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Reset code coverage line counters"
    )

    add_custom_target(${CMAKE_PROJECT_NAME}_coverage_capture
        # Capture gcov line counters
        COMMAND ${LCOV_EXECUTABLE} --quiet --directory . --capture --output-file ${COVERAGE_INFO}
        # Filter out undesirable code
        COMMAND ${LCOV_EXECUTABLE} --quiet --remove ${COVERAGE_INFO} '/usr/include/*' 'GG/utf8/*' 'src/GIL/*' 'test/*' --output-file ${COVERAGE_CLEANED}
        DEPENDS ${CMAKE_PROJECT_NAME}_coverage_clear
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Capture code coverage line counters"
    )

    add_custom_target(${CMAKE_PROJECT_NAME}_coverage_report
        # Create coverage report
        COMMAND ${GENHTML_EXECUTABLE} --quiet --output-directory coverage --title "${CMAKE_PROJECT_NAME}" ${COVERAGE_CLEANED}
        DEPENDS ${CMAKE_PROJECT_NAME}_coverage_capture
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Generate code coverage report in `coverage/index.html`"
    )

    if(NOT TARGET coverage)
        add_custom_target(coverage
            COMMENT "Create code coverage for ${CMAKE_PROJECT_NAME}")
    endif()

    add_dependencies(coverage ${CMAKE_PROJECT_NAME}_coverage_report)
endfunction()

function(ADD_COVERAGE _TARGET _TEST_TARGET)
    add_dependencies(${CMAKE_PROJECT_NAME}_coverage_capture ${_TARGET})
    add_dependencies(${CMAKE_PROJECT_NAME}_coverage_capture ${_TEST_TARGET})
endfunction()
