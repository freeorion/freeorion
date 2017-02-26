#include "ValueRefParserImpl.h"

#include "EnumParser.h"


namespace {
    struct universe_object_type_parser_rules : public enum_value_ref_rules<UniverseObjectType> {
        universe_object_type_parser_rules() :
           enum_value_ref_rules("ObjectType")
        {
            const parse::lexer& tok = parse::lexer::instance();

            variable_name
                %=   tok.ObjectType_
                ;
        }
    };
}

namespace parse {
    value_ref_rule<UniverseObjectType>& universe_object_type_value_ref()
    {
        static universe_object_type_parser_rules retval;
        return retval.expr;
    }
}
