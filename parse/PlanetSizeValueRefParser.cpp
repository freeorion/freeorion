#include "ValueRefParserImpl.h"

#include "EnumParser.h"

#include "../universe/Enums.h"


namespace {
    struct planet_size_parser_rules : public enum_value_ref_rules<PlanetSize> {
        planet_size_parser_rules() :
            enum_value_ref_rules("PlanetSize")
        {
            boost::spirit::qi::_val_type _val;

            const parse::lexer& tok = parse::lexer::instance();

            variable_name
                %=   tok.PlanetSize_
                |    tok.NextLargerPlanetSize_
                |    tok.NextSmallerPlanetSize_
                ;

            enum_expr
                =   tok.Tiny_       [ _val = SZ_TINY ]
                |   tok.Small_      [ _val = SZ_SMALL ]
                |   tok.Medium_     [ _val = SZ_MEDIUM ]
                |   tok.Large_      [ _val = SZ_LARGE ]
                |   tok.Huge_       [ _val = SZ_HUGE ]
                |   tok.Asteroids_  [ _val = SZ_ASTEROIDS ]
                |   tok.GasGiant_   [ _val = SZ_GASGIANT ]
                ;
        }
    };

    planet_size_parser_rules& get_planet_size_parser_rules()
    {
        static planet_size_parser_rules retval;
        return retval;
    }
}


namespace parse {
    template <>
    enum_rule<PlanetSize>& enum_expr<PlanetSize>()
    {
        return get_planet_size_parser_rules().enum_expr;
    }

    value_ref_rule<PlanetSize>& planet_size_value_ref()
    {
        return get_planet_size_parser_rules().expr;
    }
}
