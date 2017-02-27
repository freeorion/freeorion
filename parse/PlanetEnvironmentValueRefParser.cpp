#include "ValueRefParserImpl.h"

#include "EnumParser.h"

#include "../universe/Enums.h"


namespace {
    struct planet_environment_parser_rules : public enum_value_ref_rules<PlanetEnvironment> {
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

    planet_environment_parser_rules& get_planet_environment_parser_rules()
    {
        static planet_environment_parser_rules retval;
        return retval;
    }
}

namespace parse {
    template <>
    enum_rule<PlanetEnvironment>& enum_expr<PlanetEnvironment>()
    {
        return get_planet_environment_parser_rules().enum_expr;
    }

    value_ref_rule<PlanetEnvironment>& planet_environment_value_ref()
    {
        return get_planet_environment_parser_rules().expr;
    }
}
