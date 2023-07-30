# GiGi - A GUI for OpenGL
#
#  Copyright (C) 2010 T. Zachary Laine <whatwasthataddress@gmail.com>
#  Copyright (C) 2013-2020 The FreeOrion Project
#
# Released under the GNU Lesser General Public License 2.1 or later.
# Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
# SPDX-License-Identifier: LGPL-2.1-or-later

find_package(Boost ${MINIMUM_BOOST_VERSION} COMPONENTS unit_test_framework REQUIRED)

add_executable(gg_unittest
    main.cpp
)

target_link_libraries(gg_unittest
    GiGi
    Boost::boost
    Boost::system
    Boost::unit_test_framework
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
    if(NOT APPLE)
        add_test(
            NAME ${_TEST}
            COMMAND gg_unittest --log_level=message "--run_test=${_TEST}" --catch_system_error=yes
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        )
    endif()
endforeach()
