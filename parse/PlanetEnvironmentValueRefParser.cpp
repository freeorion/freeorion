#include "ValueRefParserImpl.h"

#include "EnumParser.h"

#include "../universe/Enums.h"


namespace {
    struct planet_environment_parser_rules :
        public parse::detail::enum_value_ref_rules<PlanetEnvironment>
    {
        planet_environment_parser_rules() :
            enum_value_ref_rules("PlanetEnvironment")
        {
            boost::spirit::qi::_val_type _val;

            const parse::lexer& tok = parse::lexer::instance();

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
    };
}


namespace parse { namespace detail {

enum_value_ref_rules<PlanetEnvironment>& planet_environment_rules()
{
    static planet_environment_parser_rules retval;
    return retval;
}

} }
