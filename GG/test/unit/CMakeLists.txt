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

add_executable(gg_unittest
    main.cpp
)

target_compile_definitions(gg_unittest
    PRIVATE
        -DBOOST_TEST_DYN_LINK
        -DBOOST_TEST_NO_LIB
)

target_link_libraries(gg_unittest
    GiGi
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARIES}
)

add_coverage(fo_unittest_parse
    unittest
)

add_dependencies(unittest
    gg_unittest
)

set(GiGi_TESTS
    TestButton
)

foreach(_TEST ${GiGi_TESTS})
    target_sources(gg_unittest
        PRIVATE
        ${_TEST}.cpp
    )
    add_test(
        NAME ${_TEST}
        COMMAND gg_unittest --log_level=message "--run_test=${_TEST}" --catch_system_error=yes
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endforeach()
