#include "ValueRefParserImpl.h"

#include "EnumParser.h"


namespace {
    struct planet_type_parser_rules {
        planet_type_parser_rules() {
            qi::_1_type _1;
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
                =    parse::planet_type_enum() [ _val = new_<ValueRef::Constant<PlanetType> >(_1) ]
                ;

            initialize_bound_variable_parser<PlanetType>(bound_variable, variable_name);

            statistic_sub_value_ref
                =   constant
                |   bound_variable
                ;

            initialize_nonnumeric_expression_parsers<PlanetType>(function_expr, operated_expr, expr, primary_expr);

            initialize_nonnumeric_statistic_parser<PlanetType>(statistic, statistic_sub_value_ref);

            primary_expr
                =   constant
                |   bound_variable
                |   statistic
                ;

            variable_name.name("PlanetType variable name (e.g., PlanetType)");
            constant.name("PlanetType");
            bound_variable.name("PlanetType variable");
            statistic_sub_value_ref.name("PlanetType statistic value reference");
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

        name_token_rule variable_name;
        parse::value_ref_rule<PlanetType> constant;
        variable_rule<PlanetType> bound_variable;
        parse::value_ref_rule<PlanetType> statistic_sub_value_ref;
        statistic_rule<PlanetType> statistic;
        expression_rule<PlanetType> function_expr;
        expression_rule<PlanetType> operated_expr;
        parse::value_ref_rule<PlanetType> expr;
        parse::value_ref_rule<PlanetType> primary_expr;
    };
}

namespace parse {
    value_ref_rule<PlanetType>& planet_type_value_ref()
    {
        static planet_type_parser_rules retval;
        return retval.expr;
    }
}
