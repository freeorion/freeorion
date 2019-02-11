#ifndef _ConditionParser7_h_
#define _ConditionParser7_h_

#include "ConditionParserImpl.h"
#include "EnumValueRefRules.h"

namespace parse { namespace detail {
    struct condition_parser_rules_7 : public condition_parser_grammar {
        condition_parser_rules_7(const parse::lexer& tok,
                                 Labeller& label,
                                 const condition_parser_grammar& condition_parser,
                                 const value_ref_grammar<std::string>& string_grammar);

        condition_parser_rule  ordered_bombarded_by;
        condition_parser_rule  contains;
        condition_parser_rule  contained_by;
        condition_parser_rule  star_type;
        rule<Condition::ContentType ()> content_type;
        condition_parser_rule  location;
        condition_parser_rule  combat_targets;
        condition_parser_rule  owner_has_shippart_available;
        condition_parser_rule  start;
        star_type_parser_rules star_type_rules;
        single_or_bracketed_repeat<value_ref_rule<StarType>> one_or_more_star_types;
    };

    }
}

#endif // _ConditionParser7_h_
