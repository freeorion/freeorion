#include "ValueRefParserImpl.h"

#include "EnumParser.h"

#include <GG/ReportParseError.h>


namespace {

    struct star_type_parser_rules
    {
        star_type_parser_rules()
            {
                qi::_1_type _1;
                qi::_a_type _a;
                qi::_val_type _val;
                using phoenix::new_;
                using phoenix::push_back;
                using phoenix::static_cast_;

                const parse::lexer& tok = parse::lexer::instance();

                final_token
                    %=   tok.StarType_
                    ;

                constant
                    =    parse::enum_parser<StarType>() [ _val = new_<ValueRef::Constant<StarType> >(_1) ]
                    ;

                variable
                    =    first_token [ push_back(_a, _1) ] > '.'
                    >   -(container_token [ push_back(_a, _1) ] > '.')
                    >    final_token [ push_back(_a, _1), _val = new_<ValueRef::Variable<StarType> >(_a) ]
                    ;

                initialize_nonnumeric_statistic_parser<StarType>(statistic, final_token);

                primary_expr
                    %=   constant
                    |    variable
                    |    statistic
                    ;

                final_token.name("StarType variable name (e.g., StarType)");
                constant.name("StarType");
                variable.name("StarType variable");
                statistic.name("StarType statistic");
                primary_expr.name("StarType expression");

#if DEBUG_VALUEREF_PARSERS
                debug(final_token);
                debug(constant);
                debug(variable);
                debug(statistic);
                debug(primary_expr);
#endif
            }

        typedef parse::value_ref_parser_rule<StarType>::type rule;
        typedef variable_rule<StarType>::type variable_rule;
        typedef statistic_rule<StarType>::type statistic_rule;

        name_token_rule final_token;
        rule constant;
        variable_rule variable;
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
