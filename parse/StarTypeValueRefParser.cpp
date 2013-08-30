#include "ValueRefParserImpl.h"

#include "EnumParser.h"


namespace {
    struct star_type_parser_rules {
        star_type_parser_rules() {
            qi::_1_type _1;
            qi::_a_type _a;
            qi::_val_type _val;
            using phoenix::new_;
            using phoenix::push_back;

            const parse::lexer& tok = parse::lexer::instance();

            variable_name
                %=   tok.StarType_
                |    tok.NextOlderStarType_
                |    tok.NextYoungerStarType_
                ;

            constant
                =    parse::enum_parser<StarType>() [ _val = new_<ValueRef::Constant<StarType> >(_1) ]
                ;

            initialize_bound_variable_parser<StarType>(bound_variable, variable_name);
            initialize_nonnumeric_statistic_parser<StarType>(statistic, variable_name);

            primary_expr
                %=   constant
                |    bound_variable
                |    statistic
                ;

            variable_name.name("StarType variable name (e.g., StarType)");
            constant.name("StarType");
            bound_variable.name("StarType variable");
            statistic.name("StarType statistic");
            primary_expr.name("StarType expression");

#if DEBUG_VALUEREF_PARSERS
            debug(variable_name);
            debug(constant);
            debug(bound_variable);
            debug(statistic);
            debug(primary_expr);
#endif
        }

        typedef parse::value_ref_parser_rule<StarType>::type rule;
        typedef variable_rule<StarType>::type variable_rule;
        typedef statistic_rule<StarType>::type statistic_rule;

        name_token_rule variable_name;
        rule constant;
        variable_rule bound_variable;
        statistic_rule statistic;
        rule primary_expr;
    };
}

namespace parse {
    template <>
    value_ref_parser_rule<StarType>::type& value_ref_parser<StarType>()
    {
        static star_type_parser_rules retval;
        return retval.primary_expr;
    }
}
