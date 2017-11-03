#ifndef _ConditionParser6_h_
#define _ConditionParser6_h_

#include "ConditionParserImpl.h"

namespace parse { namespace detail {
    struct condition_parser_rules_6 : public condition_parser_grammar {
        condition_parser_rules_6(const parse::lexer& tok,
                                 Labeller& labeller,
                                 const condition_parser_grammar& condition_parser,
                                 const value_ref_grammar<std::string>& string_grammar);

        typedef rule<
            void (std::vector<value_ref_payload<std::string>>&)
        > string_ref_vec_rule;

        typedef rule<
            condition_signature,
            boost::spirit::qi::locals<std::vector<value_ref_payload<std::string>>>
        > building_focus_species_rule;

        typedef rule<
            condition_signature,
            boost::spirit::qi::locals<std::vector<value_ref_payload<PlanetType>>>
        > planet_type_rule;

        typedef rule<
            condition_signature,
            boost::spirit::qi::locals<std::vector<value_ref_payload<PlanetSize>>>
        > planet_size_rule;

        typedef rule<
            condition_signature,
            boost::spirit::qi::locals<
                std::vector<value_ref_payload<PlanetEnvironment>>,
                value_ref_payload<std::string>
            >
        > planet_environment_rule;

        string_ref_vec_rule               string_ref_vec;
        building_focus_species_rule       homeworld;
        building_focus_species_rule       building;
        building_focus_species_rule       species;
        building_focus_species_rule       focus_type;
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
