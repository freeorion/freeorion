#include "ValueRefParserImpl.h"

#include "EnumParser.h"


namespace {
    struct star_type_parser_rules : public enum_value_ref_rules<StarType> {
        star_type_parser_rules() :
            enum_value_ref_rules("StarType")
        {
            qi::_1_type _1;
            qi::_val_type _val;
            using phoenix::new_;

            const parse::lexer& tok = parse::lexer::instance();

            variable_name
                %=   tok.StarType_
                |    tok.NextOlderStarType_
                |    tok.NextYoungerStarType_
                ;

            constant_expr
                =    parse::star_type_enum() [ _val = new_<ValueRef::Constant<StarType> >(_1) ]
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
