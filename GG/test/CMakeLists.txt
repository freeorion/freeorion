# GG is a GUI for SDL and OpenGL.
#
# Copyright (C) 2010 T. Zachary Laine
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

find_package(Boost ${MINIMUM_BOOST_VERSION} COMPONENTS unit_test_framework REQUIRED)

add_custom_target(check
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    COMMENT "Run tests for ${CMAKE_PROJECT_NAME}"
)

set(GiGi_TESTS
)

foreach(_TEST ${GiGi_TESTS})
    add_executable(test_${_TEST} EXCLUDE_FROM_ALL ${_TEST}.cpp)
    target_compile_definitions(test_${_TEST}
        PRIVATE
            -DBOOST_TEST_DYN_LINK
            -DBOOST_TEST_MODULE="${_TEST}Module"
    )
    target_link_libraries(test_${_TEST} ${Boost_LIBRARIES} GiGi)

    add_test(
        NAME ${_TEST}
        COMMAND test_${_TEST} --log_level=all --catch_system_error=yes
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
    add_dependencies(check test_${_TEST})
    add_coverage(test_${_TEST} check)
endforeach()
