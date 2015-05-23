#include "ValueRefParserImpl.h"

#include "EnumParser.h"


namespace {
    struct planet_environment_parser_rules {
        planet_environment_parser_rules() {
            qi::_1_type _1;
            qi::_val_type _val;
            using phoenix::new_;
            using phoenix::push_back;

            const parse::lexer& tok = parse::lexer::instance();

            variable_name
                %=  tok.PlanetEnvironment_
                ;

            constant
                =   parse::enum_parser<PlanetEnvironment>() [ _val = new_<ValueRef::Constant<PlanetEnvironment> >(_1) ]
                ;

            initialize_bound_variable_parser<PlanetEnvironment>(bound_variable, variable_name);

            statistic_sub_value_ref
                =   constant
                |   bound_variable
                ;

            initialize_nonnumeric_expression_parsers<PlanetEnvironment>(function_expr, operated_expr, expr, primary_expr);

            initialize_nonnumeric_statistic_parser<PlanetEnvironment>(statistic, statistic_sub_value_ref);

            primary_expr
                =   constant
                |   bound_variable
                |   statistic
                ;

            variable_name.name("PlanetEnvironment variable name (e.g., PlanetEnvironment)");
            constant.name("PlanetEnvironment");
            bound_variable.name("PlanetEnvironment variable");
            statistic.name("PlanetEnvironment statistic");
            primary_expr.name("PlanetEnvironment expression");

#if DEBUG_VALUEREF_PARSERS
            debug(variable_name);
            debug(constant);
            debug(bound_variable);
            debug(statistic);
            debug(primary_expr);
#endif
        }

        typedef parse::value_ref_parser_rule<PlanetEnvironment>::type   rule;
        typedef variable_rule<PlanetEnvironment>::type                  variable_rule;
        typedef statistic_rule<PlanetEnvironment>::type                 statistic_rule;
        typedef expression_rule<PlanetEnvironment>::type                expression_rule;

        name_token_rule variable_name;
        rule            constant;
        variable_rule   bound_variable;
        rule            statistic_sub_value_ref;
        statistic_rule  statistic;
        expression_rule function_expr;
        expression_rule operated_expr;
        rule            expr;
        rule            primary_expr;
    };
}

namespace parse {
    template <>
    value_ref_parser_rule<PlanetEnvironment>::type& value_ref_parser<PlanetEnvironment>()
    {
        static planet_environment_parser_rules retval;
        return retval.expr;
    }
}
