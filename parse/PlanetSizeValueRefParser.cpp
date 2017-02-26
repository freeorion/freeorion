#include "ValueRefParserImpl.h"

#include "EnumParser.h"


namespace {
    struct planet_size_parser_rules : public enum_value_ref_rules<PlanetSize> {
        planet_size_parser_rules() :
            enum_value_ref_rules("PlanetSize")
        {
            qi::_1_type _1;
            qi::_val_type _val;
            using phoenix::new_;

            const parse::lexer& tok = parse::lexer::instance();

            variable_name
                %=   tok.PlanetSize_
                |    tok.NextLargerPlanetSize_
                |    tok.NextSmallerPlanetSize_
                ;

            constant_expr
                =    parse::enum_expr<PlanetSize>() [ _val = new_<ValueRef::Constant<PlanetSize> >(_1) ]
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
