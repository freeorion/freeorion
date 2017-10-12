#include "ValueRefParserImpl.h"

#include "EnumParser.h"

#include "../universe/Enums.h"


namespace parse { namespace detail {
    planet_type_parser_rules::planet_type_parser_rules(
        const parse::lexer& tok,
        Labeller& labeller,
        const condition_parser_grammar& condition_parser
    ) :
        enum_value_ref_rules("PlanetType", tok, labeller, condition_parser)
    {
        boost::spirit::qi::_val_type _val;

        variable_name
            %=   tok.PlanetType_
            |    tok.OriginalType_
            |    tok.NextCloserToOriginalPlanetType_
            |    tok.NextBetterPlanetType_
            |    tok.ClockwiseNextPlanetType_
            |    tok.CounterClockwiseNextPlanetType_
            ;

        enum_expr
            =   tok.Swamp_      [ _val = PT_SWAMP ]
            |   tok.Toxic_      [ _val = PT_TOXIC ]
            |   tok.Inferno_    [ _val = PT_INFERNO ]
            |   tok.Radiated_   [ _val = PT_RADIATED ]
            |   tok.Barren_     [ _val = PT_BARREN ]
            |   tok.Tundra_     [ _val = PT_TUNDRA ]
            |   tok.Desert_     [ _val = PT_DESERT ]
            |   tok.Terran_     [ _val = PT_TERRAN ]
            |   tok.Ocean_      [ _val = PT_OCEAN ]
            |   tok.Asteroids_  [ _val = PT_ASTEROIDS ]
            |   tok.GasGiant_   [ _val = PT_GASGIANT ]
            ;
    }
} }
