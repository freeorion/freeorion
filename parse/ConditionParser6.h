#ifndef _ConditionParser6_h_
#define _ConditionParser6_h_

#include "ConditionParserImpl.h"

namespace parse { namespace detail {
    struct condition_parser_rules_6 : public condition_parser_grammar {
        condition_parser_rules_6(const parse::lexer& tok,
                                 Labeller& labeller,
                                 const condition_parser_grammar& condition_parser,
                                 const parse::value_ref_grammar<std::string>& string_grammar);

        typedef rule<
            std::vector<ValueRef::ValueRefBase<std::string>*> ()
        > string_ref_vec_rule;

        typedef rule<
            condition_signature,
            boost::spirit::qi::locals<std::vector<ValueRef::ValueRefBase<std::string>*>>
        > building_rule;

        typedef rule<
            condition_signature,
            boost::spirit::qi::locals<std::vector<ValueRef::ValueRefBase<PlanetType>*>>
        > planet_type_rule;

        typedef rule<
            condition_signature,
            boost::spirit::qi::locals<std::vector<ValueRef::ValueRefBase<PlanetSize>*>>
        > planet_size_rule;

        typedef rule<
            condition_signature,
            boost::spirit::qi::locals<
                std::vector<ValueRef::ValueRefBase<PlanetEnvironment>*>,
                ValueRef::ValueRefBase<std::string>*
            >
        > planet_environment_rule;

        string_ref_vec_rule               string_ref_vec;
        condition_parser_rule             homeworld;
        building_rule                     building;
        condition_parser_rule             species;
        condition_parser_rule             focus_type;
        planet_type_rule                  planet_type;
        planet_size_rule                  planet_size;
        planet_environment_rule           planet_environment;
        condition_parser_rule             object_type;
        condition_parser_rule             start;
        universe_object_type_parser_rules universe_object_type_rules;
        planet_type_parser_rules          planet_type_rules;
        planet_size_parser_rules          planet_size_rules;
        planet_environment_parser_rules   planet_environment_rules;
    };

    }
}

#endif // _ConditionParser6_h_
