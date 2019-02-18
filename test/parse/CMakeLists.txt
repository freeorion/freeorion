find_package(Boost ${MINIMUM_BOOST_VERSION} COMPONENTS unit_test_framework REQUIRED)

add_executable(fo_unittest_parse
    main.cpp
    CommonTest.cpp
    $<TARGET_OBJECTS:freeorionparseobj>
)

target_compile_definitions(fo_unittest_parse
    PRIVATE
        -DFREEORION_BUILD_SERVER
        -DBOOST_TEST_DYN_LINK
        -DBOOST_TEST_NO_LIB
)

target_include_directories(fo_unittest_parse
    PRIVATE
        ${PROJECT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/GG/
        ${Boost_INCLUDE_DIRS}
)

target_link_libraries(fo_unittest_parse
    freeorioncommon
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
)

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
)

foreach(_TEST ${FO_TEST_PARSE})
    target_sources(fo_unittest_parse
        PRIVATE
        ${_TEST}.cpp
    )
    add_test(
        NAME ${_TEST}
        COMMAND fo_unittest_parse --log_level=message "--run_test=${_TEST}" --catch_system_error=yes
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endforeach()
