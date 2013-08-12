cmake_minimum_required(VERSION 2.6)
cmake_policy(VERSION 2.6.4)

project(test_parsers)

message("-- Configuring test_parsers")

add_definitions(-DFREEORION_BUILD_SERVER)

add_executable(test_parsers
    ../../combat/CombatSystem.cpp
    ../../network/ServerNetworking.cpp
    ../../server/SaveLoad.cpp
    ../../server/ServerApp.cpp
    ../../server/ServerFSM.cpp
    ../../universe/UniverseServer.cpp
    ../../universe/Universe.cpp
    test.cpp
    lexer_test_rules.cpp
)

target_link_libraries(test_parsers
    freeorioncommon
    freeorionparse
    log4cpp
    ${CMAKE_THREAD_LIBS_INIT}
)

install(
    TARGETS test_parsers
    RUNTIME DESTINATION .
    COMPONENT COMPONENT_FREEORION
)

find_package (Boost REQUIRED COMPONENTS unit_test_framework)

include_directories (
    ${Boost_UNIT_TEST_FRAMEWORK_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
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

install(
    TARGETS test_parsers_boost
    RUNTIME DESTINATION .
    COMPONENT COMPONENT_FREEORION
)

add_test(enum_parser                           ${CMAKE_BINARY_DIR}/test_parsers_boost --run_test EnumParser)
add_test(value_ref_double_parser               ${CMAKE_BINARY_DIR}/test_parsers_boost --run_test ValueRefDoubleParser)
add_test(value_ref_int_parser                  ${CMAKE_BINARY_DIR}/test_parsers_boost --run_test ValueRefIntParser)
add_test(value_ref_string_parser               ${CMAKE_BINARY_DIR}/test_parsers_boost --run_test ValueRefStringParser)
add_test(value_ref_universe_object_type_parser ${CMAKE_BINARY_DIR}/test_parsers_boost --run_test ValueRefUniverseObjectTypeParser)

macro (add_test_and_data_files name)
    set(suffix "")
    set(args "")
    foreach (arg ${ARGN})
        if (EXISTS ${CMAKE_SOURCE_DIR}/parse/test/${arg})
            get_filename_component(filename_without_path ${arg} NAME)
            set(suffix ${suffix}-${filename_without_path})
            set(args "${args};${CMAKE_CURRENT_SOURCE_DIR}/${filename_without_path}")
        else ()
            set(suffix ${suffix}-${arg})
            set(args "${args};${arg}")
        endif ()
    endforeach ()
    add_test (${name}${suffix} ${CMAKE_BINARY_DIR}/${name} ${args})
endmacro ()

add_test_and_data_files(test_parsers lexer -f lexer_tokens)
add_test_and_data_files(test_parsers double_value_ref_parser -f double_variable)
add_test_and_data_files(test_parsers double_value_ref_parser -f double_variable_arithmetic)
add_test_and_data_files(test_parsers double_value_ref_parser -f double_statistic)
add_test_and_data_files(test_parsers string_value_ref_parser -f string_variable)
add_test_and_data_files(test_parsers string_value_ref_parser -f double_variable)
add_test_and_data_files(test_parsers string_value_ref_parser -f double_statistic)
add_test_and_data_files(test_parsers planet_size_value_ref_parser -f planet_size_constant)
add_test_and_data_files(test_parsers planet_size_value_ref_parser -f planet_size_variable)
add_test_and_data_files(test_parsers planet_size_value_ref_parser -f planet_size_statistic)
add_test_and_data_files(test_parsers planet_type_value_ref_parser -f planet_type_constant)
add_test_and_data_files(test_parsers planet_type_value_ref_parser -f planet_type_variable)
add_test_and_data_files(test_parsers planet_type_value_ref_parser -f planet_type_statistic)
add_test_and_data_files(test_parsers planet_environment_value_ref_parser -f planet_environment_constant)
add_test_and_data_files(test_parsers planet_environment_value_ref_parser -f planet_environment_variable)
add_test_and_data_files(test_parsers planet_environment_value_ref_parser -f planet_environment_statistic)
add_test_and_data_files(test_parsers star_type_value_ref_parser -f star_type_constant)
add_test_and_data_files(test_parsers star_type_value_ref_parser -f star_type_variable)
add_test_and_data_files(test_parsers star_type_value_ref_parser -f star_type_statistic)
add_test_and_data_files(test_parsers double_value_ref_evaluation -f double_arithmetic_evaluated)
add_test_and_data_files(test_parsers condition_parser -f condition_parser_1)
add_test_and_data_files(test_parsers condition_parser -f condition_parser_2)
add_test_and_data_files(test_parsers condition_parser -f condition_parser_3)
add_test_and_data_files(test_parsers effect_parser -f effect_parser)
add_test_and_data_files(test_parsers buildings_parser -f buildings)
add_test_and_data_files(test_parsers buildings_parser -f buildings_permutations)
add_test_and_data_files(test_parsers specials_parser -f specials)
add_test_and_data_files(test_parsers specials_parser -f specials_permutations)
add_test_and_data_files(test_parsers species_parser -f species)
add_test_and_data_files(test_parsers species_parser -f species_permutations)
add_test_and_data_files(test_parsers techs_parser -f techs)
add_test_and_data_files(test_parsers techs_parser -f techs_permutations)
add_test_and_data_files(test_parsers items_parser -f items)
add_test_and_data_files(test_parsers items_parser -f items_permutations)
add_test_and_data_files(test_parsers ship_parts_parser -f ship_parts)
add_test_and_data_files(test_parsers ship_hulls_parser -f ship_hulls)
add_test_and_data_files(test_parsers ship_designs_parser -f ship_designs)
add_test_and_data_files(test_parsers ship_designs_parser -f ship_designs_permutations)
add_test_and_data_files(test_parsers fleet_plans_parser -f fleet_plans)
add_test_and_data_files(test_parsers fleet_plans_parser -f fleet_plans_permutations)
add_test_and_data_files(test_parsers monster_fleet_plans_parser -f monster_fleet_plans)
add_test_and_data_files(test_parsers monster_fleet_plans_parser -f monster_fleet_plans_permutations)

add_test_and_data_files(test_parsers double_value_ref_parser -f double_arithmetic_errors --fail)
add_test_and_data_files(test_parsers double_value_ref_parser -f double_variable_errors --fail)
add_test_and_data_files(test_parsers double_value_ref_parser -f double_statistic_errors --fail)
add_test_and_data_files(test_parsers string_value_ref_parser -f string_variable_errors --fail)
add_test_and_data_files(test_parsers string_value_ref_parser -f string_statistic_errors --fail)
add_test_and_data_files(test_parsers planet_size_value_ref_parser -f planet_size_variable_errors --fail)
add_test_and_data_files(test_parsers planet_size_value_ref_parser -f planet_size_statistic_errors --fail)
add_test_and_data_files(test_parsers planet_type_value_ref_parser -f planet_type_variable_errors --fail)
add_test_and_data_files(test_parsers planet_type_value_ref_parser -f planet_type_statistic_errors --fail)
add_test_and_data_files(test_parsers planet_environment_value_ref_parser -f planet_environment_variable_errors --fail)
add_test_and_data_files(test_parsers planet_environment_value_ref_parser -f planet_environment_statistic_errors --fail)
add_test_and_data_files(test_parsers star_type_value_ref_parser -f star_type_variable_errors --fail)
add_test_and_data_files(test_parsers star_type_value_ref_parser -f star_type_statistic_errors --fail)
add_test_and_data_files(test_parsers condition_parser -f condition_parser_1_errors --fail)
add_test_and_data_files(test_parsers condition_parser -f condition_parser_2_errors --fail)
add_test_and_data_files(test_parsers condition_parser -f condition_parser_3_errors --fail)
add_test_and_data_files(test_parsers effect_parser -f effect_parser_errors --fail)
add_test_and_data_files(test_parsers buildings_parser -f buildings_errors --fail)
add_test_and_data_files(test_parsers specials_parser -f specials_errors --fail)
add_test_and_data_files(test_parsers species_parser -f species_errors --fail)
add_test_and_data_files(test_parsers techs_parser -f techs_errors --fail)
add_test_and_data_files(test_parsers items_parser -f items_errors --fail)
add_test_and_data_files(test_parsers ship_parts_parser -f ship_parts_errors --fail)
add_test_and_data_files(test_parsers ship_hulls_parser -f ship_hulls_errors --fail)
add_test_and_data_files(test_parsers ship_designs_parser -f ship_designs_errors --fail)
add_test_and_data_files(test_parsers fleet_plans_parser -f fleet_plans_errors --fail)
add_test_and_data_files(test_parsers monster_fleet_plans_parser -f monster_fleet_plans_errors --fail)
