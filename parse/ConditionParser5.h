#ifndef _ConditionParser5_h_
#define _ConditionParser5_h_

#include "ConditionParserImpl.h"

namespace parse { namespace detail {
    struct condition_parser_rules_5 : public condition_parser_grammar {
        condition_parser_rules_5(const parse::lexer& tok,
                                 Labeller& label,
                                 const condition_parser_grammar& condition_parser,
                                 const value_ref_grammar<std::string>& string_grammar);

        parse::int_arithmetic_rules int_rules;
        condition_parser_rule       has_special;
        condition_parser_rule       has_tag;
        condition_parser_rule       owner_has_tech;
        condition_parser_rule       design_has_hull;
        condition_parser_rule       predefined_design;
        condition_parser_rule       design_number;
        condition_parser_rule       produced_by_empire;
        condition_parser_rule       visible_to_empire;
        condition_parser_rule       explored_by_empire;
        condition_parser_rule       resupplyable_by;
        condition_parser_rule       object_id;
        condition_parser_rule       start;
    };

    }
}

#endif // _ConditionParser5_h_
