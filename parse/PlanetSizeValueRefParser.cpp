#include "ValueRefParserImpl.h"

#include "EnumParser.h"


namespace {
    struct planet_size_parser_rules : public enum_value_ref_rules<PlanetSize> {
        planet_size_parser_rules() :
            enum_value_ref_rules("PlanetSize")
        {
            const parse::lexer& tok = parse::lexer::instance();

            variable_name
                %=   tok.PlanetSize_
                |    tok.NextLargerPlanetSize_
                |    tok.NextSmallerPlanetSize_
                ;
        }
    };
}


namespace parse {
    value_ref_rule<PlanetSize>& planet_size_value_ref()
    {
        static planet_size_parser_rules retval;
        return retval.expr;
    }
}
