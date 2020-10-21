#include "ValueRefParser.h"

#include "EnumParser.h"
#include "EnumValueRefRules.h"

#include "../universe/Enums.h"

namespace parse { namespace detail {
    planet_environment_parser_rules::planet_environment_parser_rules(
        const parse::lexer& tok,
        Labeller& label,
        const condition_parser_grammar& condition_parser
    ) :
        enum_value_ref_rules("PlanetEnvironment", tok, label, condition_parser)
    {
        boost::spirit::qi::_val_type _val;

        variable_name
            %=  tok.PlanetEnvironment_
            ;

        enum_expr
            =   tok.Uninhabitable_  [ _val = PlanetEnvironment::PE_UNINHABITABLE ]
            |   tok.Hostile_        [ _val = PlanetEnvironment::PE_HOSTILE ]
            |   tok.Poor_           [ _val = PlanetEnvironment::PE_POOR ]
            |   tok.Adequate_       [ _val = PlanetEnvironment::PE_ADEQUATE ]
            |   tok.Good_           [ _val = PlanetEnvironment::PE_GOOD ]
            ;
    }
}}
