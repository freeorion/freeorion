#ifndef _ConditionParser6_h_
#define _ConditionParser6_h_

#include "ConditionParserImpl.h"
#include "EnumValueRefRules.h"

namespace parse { namespace detail {
    struct condition_parser_rules_6 : public condition_parser_grammar {
        condition_parser_rules_6(const parse::lexer& tok,
                                 Labeller& label,
                                 const condition_parser_grammar& condition_parser,
                                 const value_ref_grammar<std::string>& string_grammar);

        typedef rule<
            void (std::vector<value_ref_payload<std::string>>&)
        > string_ref_vec_rule;

        single_or_bracketed_repeat<value_ref_grammar<std::string>> one_or_more_string_values;
        condition_parser_rule             homeworld;
        condition_parser_rule             building;
        condition_parser_rule             species;
        condition_parser_rule             focus_type;
        condition_parser_rule             planet_type;
        condition_parser_rule             planet_size;
        condition_parser_rule             planet_environment;
        condition_parser_rule             object_type;
        condition_parser_rule             start;
        universe_object_type_parser_rules universe_object_type_rules;
        planet_type_parser_rules          planet_type_rules;
        planet_size_parser_rules          planet_size_rules;
        planet_environment_parser_rules   planet_environment_rules;
        single_or_bracketed_repeat<value_ref_rule<PlanetType>> one_or_more_planet_types;
        single_or_bracketed_repeat<value_ref_rule<PlanetSize>> one_or_more_planet_sizes;
        single_or_bracketed_repeat<value_ref_rule<PlanetEnvironment>> one_or_more_planet_environments;
    };

    }
}

#endif // _ConditionParser6_h_
