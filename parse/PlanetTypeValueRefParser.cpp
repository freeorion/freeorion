#include "ValueRefParserImpl.h"

#include "EnumParser.h"


namespace {
    struct planet_type_parser_rules {
        planet_type_parser_rules() {
            qi::_1_type _1;
            qi::_a_type _a;
            qi::_val_type _val;
            using phoenix::new_;
            using phoenix::push_back;

            const parse::lexer& tok = parse::lexer::instance();

            variable_name
                %=   tok.PlanetType_
                |    tok.OriginalType_
                |    tok.NextCloserToOriginalPlanetType_
                |    tok.NextBetterPlanetType_
                |    tok.ClockwiseNextPlanetType_
                |    tok.CounterClockwiseNextPlanetType_
                ;

            constant
                =    parse::enum_parser<PlanetType>() [ _val = new_<ValueRef::Constant<PlanetType> >(_1) ]
                ;

            initialize_bound_variable_parser<PlanetType>(bound_variable, variable_name);
            initialize_nonnumeric_statistic_parser<PlanetType>(statistic, variable_name);

            primary_expr
                %=   constant
                |    bound_variable
                |    statistic
                ;

            variable_name.name("PlanetType variable name (e.g., PlanetType)");
            constant.name("PlanetType");
            bound_variable.name("PlanetType variable");
            statistic.name("PlanetType statistic");
            primary_expr.name("PlanetType expression");

#if DEBUG_VALUEREF_PARSERS
            debug(variable_name);
            debug(constant);
            debug(bound_variable);
            debug(statistic);
            debug(primary_expr);
#endif
        }

        typedef parse::value_ref_parser_rule<PlanetType>::type rule;
        typedef variable_rule<PlanetType>::type variable_rule;
        typedef statistic_rule<PlanetType>::type statistic_rule;

        name_token_rule variable_name;
        rule constant;
        variable_rule bound_variable;
        statistic_rule statistic;
        rule primary_expr;
    };
}

namespace parse {
    template <>
    value_ref_parser_rule<PlanetType>::type& value_ref_parser<PlanetType>()
    {
        static planet_type_parser_rules retval;
        return retval.primary_expr;
    }
}
