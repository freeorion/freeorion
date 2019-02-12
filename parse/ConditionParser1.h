#ifndef _ConditionParser1_h_
#define _ConditionParser1_h_

#include "ConditionParserImpl.h"

namespace parse { namespace detail {

    struct condition_parser_rules_1 : public condition_parser_grammar {
        condition_parser_rules_1(const parse::lexer& tok,
                                 Labeller& label,
                                 const condition_parser_grammar& condition_parser,
                                 const value_ref_grammar<std::string>& string_grammar);

        parse::int_arithmetic_rules            int_rules;
        parse::empire_affiliation_enum_grammar empire_affiliation_type_enum;
        condition_parser_rule                  all;
        condition_parser_rule                  none;
        condition_parser_rule                  source;
        condition_parser_rule                  root_candidate;
        condition_parser_rule                  target;
        condition_parser_rule                  stationary;
        condition_parser_rule                  aggressive;
        condition_parser_rule                  can_colonize;
        condition_parser_rule                  can_produce_ships;
        condition_parser_rule                  capital;
        condition_parser_rule                  monster;
        condition_parser_rule                  armed;
        condition_parser_rule                  owned_by_1;
        condition_parser_rule                  owned_by_2;
        condition_parser_rule                  owned_by_3;
        condition_parser_rule                  owned_by_4;
        condition_parser_rule                  owned_by_5;
        condition_parser_rule                  owned_by;
        condition_parser_rule                  and_;
        condition_parser_rule                  or_;
        condition_parser_rule                  not_;
        condition_parser_rule                  ordered_alternatives_of;
        condition_parser_rule                  described;
        condition_parser_rule                  start;
    };

    }
}

#endif // _ConditionParser1_h_
