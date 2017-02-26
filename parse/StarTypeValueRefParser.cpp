#include "ValueRefParserImpl.h"

#include "EnumParser.h"


namespace {
    struct star_type_parser_rules : public enum_value_ref_rules<StarType> {
        star_type_parser_rules() :
            enum_value_ref_rules("StarType")
        {
            const parse::lexer& tok = parse::lexer::instance();

            variable_name
                %=   tok.StarType_
                |    tok.NextOlderStarType_
                |    tok.NextYoungerStarType_
                ;
        }
    };
}

namespace parse {
    value_ref_rule<StarType>& star_type_value_ref()
    {
        static star_type_parser_rules retval;
        return retval.expr;
    }
}
