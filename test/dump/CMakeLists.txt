find_package(Boost ${MINIMUM_BOOST_VERSION} COMPONENTS unit_test_framework REQUIRED)

add_executable(fo_unittest_dump
    main.cpp
    $<TARGET_OBJECTS:freeorionparseobj>
)

target_compile_definitions(fo_unittest_parse
    PRIVATE
        -DFREEORION_BUILD_SERVER
)

target_include_directories(fo_unittest_dump
    PRIVATE
        ${PROJECT_SOURCE_DIR}
)

target_link_libraries(fo_unittest_dump
    freeorioncommon
    freeorionparse
    Threads::Threads
    Boost::boost
    Boost::disable_autolinking
    Boost::dynamic_linking
    Boost::unit_test_framework
)

target_dependencies_copy_to_build(fo_unittest_dump)

add_coverage(fo_unittest_dump
    unittest
)

add_dependencies(unittest
    fo_unittest_dump
)

set(FO_TEST_DUMP
    TestValueRefs
)

foreach(_TEST ${FO_TEST_DUMP})
    target_sources(fo_unittest_dump
        PRIVATE
        ${_TEST}.cpp
    )
    add_test(
        NAME ${_TEST}
        COMMAND fo_unittest_dump --log_level=all "--run_test=${_TEST}" --catch_system_error=yes
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endforeach()

