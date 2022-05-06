#include "ValueRefParser.h"

#include "EnumParser.h"
#include "EnumValueRefRules.h"

#include "../universe/Planet.h"

namespace parse::detail {
    planet_type_parser_rules::planet_type_parser_rules(
        const parse::lexer& tok,
        Labeller& label,
        const condition_parser_grammar& condition_parser
    ) :
        enum_value_ref_rules("PlanetType", tok, label, condition_parser)
    {
        boost::spirit::qi::_val_type _val;

        variable_name
            %=   tok.PlanetType_
            |    tok.OriginalType_
            |    tok.NextCloserToOriginalPlanetType_
            |    tok.NextBestPlanetType_
            |    tok.NextBetterPlanetType_
            |    tok.ClockwiseNextPlanetType_
            |    tok.CounterClockwiseNextPlanetType_
            ;

        enum_expr
            =   tok.Swamp_      [ _val = PlanetType::PT_SWAMP ]
            |   tok.Toxic_      [ _val = PlanetType::PT_TOXIC ]
            |   tok.Inferno_    [ _val = PlanetType::PT_INFERNO ]
            |   tok.Radiated_   [ _val = PlanetType::PT_RADIATED ]
            |   tok.Barren_     [ _val = PlanetType::PT_BARREN ]
            |   tok.Tundra_     [ _val = PlanetType::PT_TUNDRA ]
            |   tok.Desert_     [ _val = PlanetType::PT_DESERT ]
            |   tok.Terran_     [ _val = PlanetType::PT_TERRAN ]
            |   tok.Ocean_      [ _val = PlanetType::PT_OCEAN ]
            |   tok.Asteroids_  [ _val = PlanetType::PT_ASTEROIDS ]
            |   tok.GasGiant_   [ _val = PlanetType::PT_GASGIANT ]
            ;
    }
}
