#ifndef _ConditionParser7_h_
#define _ConditionParser7_h_

#include "ConditionParserImpl.h"
#include "EnumValueRefRules.h"

namespace parse { namespace detail {
    struct condition_parser_rules_7 : public condition_parser_grammar {
        condition_parser_rules_7(const parse::lexer& tok,
                                 Labeller& labeller,
                                 const condition_parser_grammar& condition_parser,
                                 const value_ref_grammar<std::string>& string_grammar);

        using star_type_vec_rule = rule<condition_signature>;

        typedef rule<
            condition_signature,
            boost::spirit::qi::locals<
                Condition::ContentType,
                value_ref_payload<std::string>,
                value_ref_payload<std::string>
            >
        > string_ref_rule;

        condition_parser_rule  ordered_bombarded_by;
        condition_parser_rule  contains;
        condition_parser_rule  contained_by;
        star_type_vec_rule     star_type;
        string_ref_rule        location;
        condition_parser_rule  owner_has_shippart_available;
        condition_parser_rule  start;
        star_type_parser_rules star_type_rules;
        single_or_bracketed_repeat<value_ref_rule<StarType>> one_or_more_star_types;
    };

    }
}

#endif // _ConditionParser7_h_
