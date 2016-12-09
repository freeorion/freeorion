find_package(Boost ${MINIMUM_BOOST_VERSION} COMPONENTS unit_test_framework REQUIRED)

set(GiGi_TESTS
)

foreach(_TEST ${GiGi_TESTS})
    add_executable(test_${_TEST} ${_TEST}.cpp)
    set_target_properties(test_${_TEST}
        PROPERTIES
        CXX_STANDARD_REQUIRED ON
        CXX_STANDARD 11
    )
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
endforeach()
