#ifndef _ConditionParser3_h_
#define _ConditionParser3_h_

#include "ConditionParserImpl.h"

namespace parse { namespace detail {
    struct condition_parser_rules_3 : public condition_parser_grammar {
        condition_parser_rules_3(const parse::lexer& tok,
                                 Labeller& labeller,
                                 const condition_parser_grammar& condition_parser,
                                 const value_ref_grammar<std::string>& string_grammar);

        typedef rule<
            condition_signature,
            boost::spirit::qi::locals<
                value_ref_payload<double>,
                value_ref_payload<double>,
                value_ref_payload<std::string>,
                Condition::ComparisonType,
                Condition::ComparisonType,
                value_ref_payload<std::string>,
                value_ref_payload<int>,
                value_ref_payload<int>
            >
        > double_ref_double_ref_rule;

        typedef rule<
            condition_signature,
            boost::spirit::qi::locals<
                value_ref_payload<int>,
                value_ref_payload<int>
            >
        > int_ref_int_ref_rule;

        typedef rule<
            condition_signature,
            boost::spirit::qi::locals<
                value_ref_payload<int>,
                Condition::SortingMethod,
                value_ref_payload<double>
            >
        > int_ref_sorting_method_double_ref_rule;

        typedef rule<
            condition_signature,
            boost::spirit::qi::locals<
                ResourceType,
                value_ref_payload<double>
            >
        > resource_type_double_ref_rule;

        parse::int_arithmetic_rules            int_rules;
        parse::castable_as_int_parser_rules    castable_int_rules;
        parse::double_parser_rules             double_rules;
        double_ref_double_ref_rule             has_special_capacity;
        double_ref_double_ref_rule             within_distance;
        int_ref_int_ref_rule                   within_starlane_jumps;
        int_ref_int_ref_rule                   number;
        double_ref_double_ref_rule             comparison_binary_double;
        double_ref_double_ref_rule             comparison_trinary_double;
        double_ref_double_ref_rule             comparison_binary_int;
        double_ref_double_ref_rule             comparison_trinary_int;
        double_ref_double_ref_rule             comparison_binary_string;
        double_ref_double_ref_rule             comparison_trinary_string;
        double_ref_double_ref_rule             value_test_2;
        double_ref_double_ref_rule             value_test_3;
        double_ref_double_ref_rule             value_test_4;
        double_ref_double_ref_rule             value_test_5;
        double_ref_double_ref_rule             value_test_6;
        int_ref_int_ref_rule                   turn;
        int_ref_int_ref_rule                   created_on_turn;
        int_ref_sorting_method_double_ref_rule number_of;
        int_ref_sorting_method_double_ref_rule number_of1;
        int_ref_sorting_method_double_ref_rule number_of2;
        condition_parser_rule                  random;
        resource_type_double_ref_rule          owner_stockpile;
        int_ref_int_ref_rule                   resource_supply_connected;
        condition_parser_rule                  can_add_starlane;
        condition_parser_rule                  start;
    };

    }
}

#endif // _ConditionParser3_h_
