find_package(Boost ${MINIMUM_BOOST_VERSION} COMPONENTS unit_test_framework REQUIRED)

add_executable(fo_unittest_parse
    main.cpp
    CommonTest.cpp
    ParserAppFixture.cpp
    $<TARGET_OBJECTS:freeorionparseobj>
)

target_compile_definitions(fo_unittest_parse
    PRIVATE
        -DFREEORION_BUILD_SERVER
)

target_include_directories(fo_unittest_parse
    PRIVATE
        ${PROJECT_SOURCE_DIR}
)

target_link_libraries(fo_unittest_parse
    freeorioncommon
    freeorionparse
    Threads::Threads
    Boost::boost
    Boost::disable_autolinking
    Boost::dynamic_linking
    Boost::unit_test_framework
)

target_dependencies_copy_to_build(fo_unittest_parse)
target_dependent_data_symlink_to_build(fo_unittest_parse ${PROJECT_SOURCE_DIR}/test-scripting)
target_dependent_data_symlink_to_build(fo_unittest_parse ${PROJECT_SOURCE_DIR}/default)

add_coverage(fo_unittest_parse
    unittest
)

add_dependencies(unittest
    fo_unittest_parse
)

set(FO_TEST_PARSE
    TestEnumParser
    #TestValueRefDoubleParser
    #TestValueRefIntParser
    #TestValueRefStringParser
    #TestValueRefUniverseObjectTypeParser
    #TestScriptParser
    TestPythonParser
    TestDefaultPythonParser
)

foreach(_TEST ${FO_TEST_PARSE})
    target_sources(fo_unittest_parse
        PRIVATE
        ${_TEST}.cpp
    )
    add_test(
        NAME ${_TEST}
        COMMAND fo_unittest_parse --log_level=all "--run_test=${_TEST}" --catch_system_error=yes
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endforeach()
