#include "ValueRefParserImpl.h"

#include "EnumParser.h"


namespace {
    struct planet_type_parser_rules : public enum_value_ref_rules<PlanetType> {
        planet_type_parser_rules() :
            enum_value_ref_rules("PlanetType")
        {
            const parse::lexer& tok = parse::lexer::instance();

            variable_name
                %=   tok.PlanetType_
                |    tok.OriginalType_
                |    tok.NextCloserToOriginalPlanetType_
                |    tok.NextBetterPlanetType_
                |    tok.ClockwiseNextPlanetType_
                |    tok.CounterClockwiseNextPlanetType_
                ;
        }
    };
}

namespace parse {
    value_ref_rule<PlanetType>& planet_type_value_ref()
    {
        static planet_type_parser_rules retval;
        return retval.expr;
    }
}
