// -*- C++ -*-
#include "../Lexer.h"
#include "../ValueRefParser.h"

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>


struct lexer_test_rules
{
    lexer_test_rules();

    typedef boost::spirit::qi::rule<
        parse::token_iterator,
        parse::skipper_type
    > test_rule;

    test_rule lexer;
    test_rule lexer_1;
    test_rule lexer_2;
    test_rule lexer_3;
};

enum test_type {
    unknown,
    lexer,
    planet_size_enum_parser,
    planet_type_enum_parser,
    planet_environment_enum_parser,
    universe_object_type_enum_parser,
    star_type_enum_parser,
    non_ship_part_meter_type_enum_parser,
    set_non_ship_part_meter_type_enum_parser,
    set_ship_part_meter_type_enum_parser,
    empire_affiliation_type_enum_parser,
    unlockable_item_type_enum_parser,
    tech_type_enum_parser,
    ship_slot_type_enum_parser,
    ship_part_class_enum_parser,
    combat_fighter_type_enum_parser,
    capture_result_enum_parser,
    value_ref_statistic_type_enum_parser,
    int_value_ref_parser,
    double_value_ref_parser,
    string_value_ref_parser,
    planet_size_value_ref_parser,
    planet_type_value_ref_parser,
    planet_environment_value_ref_parser,
    universe_object_type_value_ref_parser,
    star_type_value_ref_parser,
    int_value_ref_evaluation,
    double_value_ref_evaluation,
    condition_parser,
    effect_parser,
    buildings_parser,
    specials_parser,
    species_parser,
    techs_parser,
    items_parser,
    ship_parts_parser,
    ship_hulls_parser,
    ship_designs_parser,
    fleet_plans_parser,
    monster_fleet_plans_parser,
    alignments_parser
};

void print_help();
