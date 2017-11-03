#ifndef _ConditionParser2_h_
#define _ConditionParser2_h_

#include "ConditionParserImpl.h"

namespace parse { namespace detail {
    struct condition_parser_rules_2 : public condition_parser_grammar {
        condition_parser_rules_2(const parse::lexer& tok,
                                 Labeller& labeller,
                                 const condition_parser_grammar& condition_parser,
                                 const value_ref_grammar<std::string>& string_grammar);

        typedef rule<
            condition_signature,
            boost::spirit::qi::locals<
                value_ref_payload<int>,
                value_ref_payload<int>,
                value_ref_payload<int>,
                value_ref_payload<int>,
                value_ref_payload<std::string>
            >
        > common_rule;

        parse::int_arithmetic_rules         int_rules;
        parse::castable_as_int_parser_rules castable_int_rules;
        parse::ship_part_class_enum_grammar ship_part_class_enum;
        common_rule                         has_special_since_turn;
        common_rule                         enqueued;
        common_rule                         enqueued1;
        common_rule                         enqueued2;
        common_rule                         enqueued3;
        common_rule                         enqueued4;
        common_rule                         design_has_part;
        common_rule                         design_has_part_class;
        common_rule                         in_system;
        condition_parser_rule               start;
    };

    }
}

#endif // _ConditionParser2_h_
