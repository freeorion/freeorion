find_package(Boost ${MINIMUM_BOOST_VERSION} COMPONENTS unit_test_framework REQUIRED)

add_executable(fo_unittest_util
    main.cpp
)

target_include_directories(fo_unittest_util
    PRIVATE
        ${PROJECT_SOURCE_DIR}
)

target_link_libraries(fo_unittest_util
    Threads::Threads
    Boost::boost
    Boost::disable_autolinking
    Boost::dynamic_linking
    Boost::unit_test_framework
)

target_dependencies_copy_to_build(fo_unittest_util)

add_dependencies(unittest
    fo_unittest_util
)

set(FO_TEST_UTIL
)

foreach(_TEST ${FO_TEST_UTIL})
    target_sources(fo_unittest_util
        PRIVATE
        ${_TEST}.cpp
    )
    add_test(
        NAME ${_TEST}
        COMMAND fo_unittest_util --log_level=all "--run_test=${_TEST}" --catch_system_error=yes
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endforeach()
