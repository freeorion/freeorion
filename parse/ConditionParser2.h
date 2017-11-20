#ifndef _ConditionParser2_h_
#define _ConditionParser2_h_

#include "ConditionParserImpl.h"

namespace parse { namespace detail {
    struct condition_parser_rules_2 : public condition_parser_grammar {
        condition_parser_rules_2(const parse::lexer& tok,
                                 Labeller& label,
                                 const condition_parser_grammar& condition_parser,
                                 const value_ref_grammar<std::string>& string_grammar);

        parse::int_arithmetic_rules         int_rules;
        parse::castable_as_int_parser_rules castable_int_rules;
        parse::ship_part_class_enum_grammar ship_part_class_enum;
        condition_parser_rule               has_special_since_turn;
        condition_parser_rule               enqueued;
        condition_parser_rule               enqueued1;
        condition_parser_rule               enqueued2;
        condition_parser_rule               enqueued3;
        condition_parser_rule               enqueued4;
        condition_parser_rule               design_has_part;
        condition_parser_rule               design_has_part_class;
        condition_parser_rule               in_system;
        condition_parser_rule               start;
    };

    }
}

#endif // _ConditionParser2_h_
