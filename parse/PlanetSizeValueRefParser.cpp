#include "ValueRefParser.h"

#include "EnumParser.h"
#include "EnumValueRefRules.h"

#include "../universe/Enums.h"
#include "../universe/ValueRef.h"

namespace parse { namespace detail {
    planet_size_parser_rules::planet_size_parser_rules(
        const parse::lexer& tok,
        Labeller& label,
        const condition_parser_grammar& condition_parser
    ) :
        enum_value_ref_rules("PlanetSize", tok, label, condition_parser)
    {
        boost::spirit::qi::_val_type _val;

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
} }
