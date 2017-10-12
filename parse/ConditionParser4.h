#ifndef _ConditionParser4_h_
#define _ConditionParser4_h_

#include "ConditionParserImpl.h"

namespace parse { namespace detail {
    struct condition_parser_rules_4 : public condition_parser_grammar {
        condition_parser_rules_4(const parse::lexer& tok,
                                 Labeller& labeller,
                                 const condition_parser_grammar& condition_parser,
                                 const parse::value_ref_grammar<std::string>& string_grammar);

        typedef rule<
            condition_signature,
            boost::spirit::qi::locals<
                MeterType,
                ValueRef::ValueRefBase<double>*,
                ValueRef::ValueRefBase<double>*,
                std::string,
                ValueRef::ValueRefBase<std::string>*
            >
        > meter_value_rule;

        typedef rule<
            condition_signature,
            boost::spirit::qi::locals<
                std::string,
                ValueRef::ValueRefBase<int>*,
                ValueRef::ValueRefBase<double>*,
                ValueRef::ValueRefBase<double>*
            >
        > empire_meter_value_rule;

        parse::int_arithmetic_rules             int_rules;
        parse::double_parser_rules              double_rules;
        parse::non_ship_part_meter_enum_grammar non_ship_part_meter_type_enum;
        parse::ship_part_meter_enum_grammar     ship_part_meter_type_enum;
        meter_value_rule                        meter_value;
        meter_value_rule                        ship_part_meter_value;
        empire_meter_value_rule                 empire_meter_value;
        empire_meter_value_rule                 empire_meter_value1;
        empire_meter_value_rule                 empire_meter_value2;
        condition_parser_rule                   start;
    };

    }
}

#endif // _ConditionParser4_h_
