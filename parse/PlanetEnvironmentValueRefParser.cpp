#include "ValueRefParserImpl.h"

#include "EnumParser.h"


namespace {
    struct planet_environment_parser_rules : public enum_value_ref_rules<PlanetEnvironment> {
        planet_environment_parser_rules() :
            enum_value_ref_rules("PlanetEnvironment")
        {
            const parse::lexer& tok = parse::lexer::instance();

            variable_name
                %=  tok.PlanetEnvironment_
                ;
        }
    };
}

namespace parse {
    value_ref_rule<PlanetEnvironment>& planet_environment_value_ref()
    {
        static planet_environment_parser_rules retval;
        return retval.expr;
    }
}
