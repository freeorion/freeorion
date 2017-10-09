#include "ValueRefParserImpl.h"

#include "EnumParser.h"

#include "../universe/Enums.h"


namespace parse { namespace detail {
    planet_environment_parser_rules::planet_environment_parser_rules(
        const parse::lexer& tok,
        Labeller& labeller,
        const condition_parser_grammar& condition_parser
    ) :
        enum_value_ref_rules("PlanetEnvironment", tok, labeller, condition_parser)
    {
        boost::spirit::qi::_val_type _val;

        variable_name
            %=  tok.PlanetEnvironment_
            ;

        enum_expr
            =   tok.Uninhabitable_  [ _val = PE_UNINHABITABLE ]
            |   tok.Hostile_        [ _val = PE_HOSTILE ]
            |   tok.Poor_           [ _val = PE_POOR ]
            |   tok.Adequate_       [ _val = PE_ADEQUATE ]
            |   tok.Good_           [ _val = PE_GOOD ]
            ;
    }
}}
