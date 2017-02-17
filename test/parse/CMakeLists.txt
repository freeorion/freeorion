find_package (Boost REQUIRED COMPONENTS unit_test_framework)

add_definitions(-DFREEORION_BUILD_SERVER)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../../
)

include_directories(
    SYSTEM
    ${Boost_UNIT_TEST_FRAMEWORK_INCLUDES}
)

add_executable(test_parsers_boost
    testmain.cpp
    CommonTest.cpp
    TestEnumParser.cpp
    TestValueRefDoubleParser.cpp
    TestValueRefIntParser.cpp
    TestValueRefStringParser.cpp
    TestValueRefUniverseObjectTypeParser.cpp
)

target_link_libraries(test_parsers_boost
    freeorioncommon
    freeorionparse
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARIES}
)

add_test(enum_parser                           ${CMAKE_BINARY_DIR}/test_parsers_boost --run_test EnumParser)
add_test(value_ref_double_parser               ${CMAKE_BINARY_DIR}/test_parsers_boost --run_test ValueRefDoubleParser)
add_test(value_ref_int_parser                  ${CMAKE_BINARY_DIR}/test_parsers_boost --run_test ValueRefIntParser)
add_test(value_ref_string_parser               ${CMAKE_BINARY_DIR}/test_parsers_boost --run_test ValueRefStringParser)
add_test(value_ref_universe_object_type_parser ${CMAKE_BINARY_DIR}/test_parsers_boost --run_test ValueRefUniverseObjectTypeParser)
