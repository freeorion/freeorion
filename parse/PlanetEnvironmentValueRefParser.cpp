#include "ValueRefParserImpl.h"

#include "EnumParser.h"


namespace {
    struct planet_environment_parser_rules {
        planet_environment_parser_rules() {
            qi::_1_type _1;
            qi::_a_type _a;
            qi::_val_type _val;
            using phoenix::new_;
            using phoenix::push_back;

            const parse::lexer& tok = parse::lexer::instance();

            variable_name
                %=   tok.PlanetEnvironment_
                ;

            constant
                =    parse::enum_parser<PlanetEnvironment>() [ _val = new_<ValueRef::Constant<PlanetEnvironment> >(_1) ]
                ;

            initialize_bound_variable_parser<PlanetEnvironment>(bound_variable, variable_name);
            initialize_nonnumeric_statistic_parser<PlanetEnvironment>(statistic, variable_name);

            primary_expr
                %=   constant
                |    bound_variable
                |    statistic
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

        typedef parse::value_ref_parser_rule<PlanetEnvironment>::type rule;
        typedef variable_rule<PlanetEnvironment>::type variable_rule;
        typedef statistic_rule<PlanetEnvironment>::type statistic_rule;

        name_token_rule variable_name;
        rule constant;
        variable_rule bound_variable;
        statistic_rule statistic;
        rule primary_expr;
    };
}

namespace parse {
    template <>
    value_ref_parser_rule<PlanetEnvironment>::type& value_ref_parser<PlanetEnvironment>()
    {
        static planet_environment_parser_rules retval;
        return retval.primary_expr;
    }
}
