#include "ValueRefParserImpl.h"

#include "EnumParser.h"


namespace {
    struct universe_object_type_parser_rules : public enum_value_ref_rules<UniverseObjectType> {
        universe_object_type_parser_rules() :
           enum_value_ref_rules("ObjectType")
        {
            qi::_1_type _1;
            qi::_val_type _val;
            using phoenix::new_;

            const parse::lexer& tok = parse::lexer::instance();

            variable_name
                %=   tok.ObjectType_
                ;

            constant_expr
                =    parse::universe_object_type_enum() [ _val = new_<ValueRef::Constant<UniverseObjectType> >(_1) ]
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
