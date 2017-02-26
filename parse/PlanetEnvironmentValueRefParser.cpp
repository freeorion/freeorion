#include "ValueRefParserImpl.h"

#include "EnumParser.h"


namespace {
    struct planet_environment_parser_rules : public enum_value_ref_rules<PlanetEnvironment> {
        planet_environment_parser_rules() :
            enum_value_ref_rules("PlanetEnvironment")
        {
            qi::_1_type _1;
            qi::_val_type _val;
            using phoenix::new_;

            const parse::lexer& tok = parse::lexer::instance();

            variable_name
                %=  tok.PlanetEnvironment_
                ;

            constant_expr
                =   parse::enum_expr<PlanetEnvironment>() [ _val = new_<ValueRef::Constant<PlanetEnvironment> >(_1) ]
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
