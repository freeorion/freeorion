#include "ValueRefParserImpl.h"

#include "EnumParser.h"


namespace {
    struct planet_type_parser_rules : public enum_value_ref_rules<PlanetType> {
        planet_type_parser_rules() :
            enum_value_ref_rules("PlanetType")
        {
            qi::_1_type _1;
            qi::_val_type _val;
            using phoenix::new_;

            const parse::lexer& tok = parse::lexer::instance();

            variable_name
                %=   tok.PlanetType_
                |    tok.OriginalType_
                |    tok.NextCloserToOriginalPlanetType_
                |    tok.NextBetterPlanetType_
                |    tok.ClockwiseNextPlanetType_
                |    tok.CounterClockwiseNextPlanetType_
                ;

            constant_expr
                =    parse::enum_expr<PlanetType>() [ _val = new_<ValueRef::Constant<PlanetType> >(_1) ]
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
