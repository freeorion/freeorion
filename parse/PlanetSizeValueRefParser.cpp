#include "ValueRefParserImpl.h"

#include "EnumParser.h"


namespace {
    struct planet_size_parser_rules {
        planet_size_parser_rules() {
            qi::_1_type _1;
            qi::_a_type _a;
            qi::_val_type _val;
            using phoenix::new_;
            using phoenix::push_back;

            const parse::lexer& tok = parse::lexer::instance();

            variable_name
                %=   tok.PlanetSize_
                |    tok.NextLargerPlanetSize_
                |    tok.NextSmallerPlanetSize_
                ;

            constant
                =    parse::enum_parser<PlanetSize>() [ _val = new_<ValueRef::Constant<PlanetSize> >(_1) ]
                ;

            initialize_bound_variable_parser<PlanetSize>(bound_variable, variable_name);
            initialize_nonnumeric_statistic_parser<PlanetSize>(statistic, variable_name);

            primary_expr
                %=   constant
                |    bound_variable
                |    statistic
                ;

            variable_name.name("PlanetSize variable name (e.g., PlanetSize)");
            constant.name("PlanetSize");
            bound_variable.name("PlanetSize variable");
            statistic.name("PlanetSize statistic");
            primary_expr.name("PlanetSize expression");

#if DEBUG_VALUEREF_PARSERS
            debug(variable_name);
            debug(constant);
            debug(bound_variable);
            debug(statistic);
            debug(primary_expr);
#endif
        }

        typedef parse::value_ref_parser_rule<PlanetSize>::type rule;
        typedef variable_rule<PlanetSize>::type variable_rule;
        typedef statistic_rule<PlanetSize>::type statistic_rule;

        name_token_rule variable_name;
        rule constant;
        variable_rule bound_variable;
        statistic_rule statistic;
        rule primary_expr;
    };
}


namespace parse {

    template <>
    value_ref_parser_rule<PlanetSize>::type& value_ref_parser<PlanetSize>()
    {
        static planet_size_parser_rules retval;
        return retval.primary_expr;
    }

}
