#ifndef _ConditionParser4_h_
#define _ConditionParser4_h_

#include "ConditionParserImpl.h"

namespace parse { namespace detail {
    struct condition_parser_rules_4 : public condition_parser_grammar {
        condition_parser_rules_4(const parse::lexer& tok,
                                 Labeller& label,
                                 const condition_parser_grammar& condition_parser,
                                 const value_ref_grammar<std::string>& string_grammar);

        parse::int_arithmetic_rules             int_rules;
        parse::double_parser_rules              double_rules;
        parse::non_ship_part_meter_enum_grammar non_ship_part_meter_type_enum;
        parse::ship_part_meter_enum_grammar     ship_part_meter_type_enum;
        condition_parser_rule                   meter_value;
        condition_parser_rule                   ship_part_meter_value;
        condition_parser_rule                   empire_meter_value;
        condition_parser_rule                   empire_meter_value1;
        condition_parser_rule                   empire_meter_value2;
        condition_parser_rule                   start;
    };

    }
}

#endif // _ConditionParser4_h_
