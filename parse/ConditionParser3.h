#ifndef _ConditionParser3_h_
#define _ConditionParser3_h_

#include "ConditionParserImpl.h"

namespace parse { namespace detail {
    struct condition_parser_rules_3 : public condition_parser_grammar {
        condition_parser_rules_3(const parse::lexer& tok,
                                 Labeller& label,
                                 const condition_parser_grammar& condition_parser,
                                 const value_ref_grammar<std::string>& string_grammar);

        using comparison_operator_rule = rule<Condition::ComparisonType ()>;
        using sorting_operator_rule = rule<Condition::SortingMethod ()>;

        parse::int_arithmetic_rules         int_rules;
        parse::castable_as_int_parser_rules castable_int_rules;
        parse::double_parser_rules          double_rules;
        parse::resource_type_grammar        resource_type_enum;
        condition_parser_rule               has_special_capacity;
        condition_parser_rule               within_distance;
        condition_parser_rule               within_starlane_jumps;
        condition_parser_rule               number;
        comparison_operator_rule            comparison_operator;
        comparison_operator_rule            string_comparison_operator;
        condition_parser_rule               comparison_binary_double;
        condition_parser_rule               comparison_trinary_double;
        condition_parser_rule               comparison_binary_int;
        condition_parser_rule               comparison_trinary_int;
        condition_parser_rule               comparison_binary_string;
        condition_parser_rule               comparison_trinary_string;
        condition_parser_rule               turn;
        condition_parser_rule               created_on_turn;
        sorting_operator_rule               sorting_operator;
        condition_parser_rule               number_of;
        condition_parser_rule               number_of1;
        condition_parser_rule               number_of2;
        condition_parser_rule               unique_of1;
        condition_parser_rule               unique_of2;
        condition_parser_rule               random;
        condition_parser_rule               stockpile;
        condition_parser_rule               resource_supply_connected;
        condition_parser_rule               has_starlane_to;
        condition_parser_rule               starlane_to_would_cross_existing_starlane;
        condition_parser_rule               starlane_to_would_be_angularly_close_to_existing_starlane;
        condition_parser_rule               starlane_to_would_be_close_to_object;
        condition_parser_rule               start;
    };

    }
}


#endif
