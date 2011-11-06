#include "ValueRefParserImpl.h"

#include "EnumParser.h"

#include <GG/ReportParseError.h>


namespace {

    struct planet_type_parser_rules
    {
        planet_type_parser_rules()
            {
                qi::_1_type _1;
                qi::_a_type _a;
                qi::_val_type _val;
                using phoenix::new_;
                using phoenix::push_back;
                using phoenix::static_cast_;

                const parse::lexer& tok = parse::lexer::instance();

                final_token
                    %=   tok.PlanetType_
                    |    tok.NextBetterPlanetType_
                    ;

                constant
                    =    parse::enum_parser<PlanetType>() [ _val = new_<ValueRef::Constant<PlanetType> >(_1) ]
                    ;

                variable
                    =    first_token [ push_back(_a, _1) ] > '.'
                    >   -(container_token [ push_back(_a, _1) ] > '.')
                    >    final_token [ push_back(_a, _1), _val = new_<ValueRef::Variable<PlanetType> >(_a) ]
                    ;

                initialize_nonnumeric_statistic_parser<PlanetType>(statistic, final_token);

                primary_expr
                    %=   constant
                    |    variable
                    |    statistic
                    ;

                final_token.name("PlanetType variable name (e.g., PlanetType)");
                constant.name("PlanetType");
                variable.name("PlanetType variable");
                statistic.name("PlanetType statistic");
                primary_expr.name("PlanetType expression");

#if DEBUG_VALUEREF_PARSERS
                debug(final_token);
                debug(constant);
                debug(variable);
                debug(statistic);
                debug(primary_expr);
#endif
            }

        typedef parse::value_ref_parser_rule<PlanetType>::type rule;
        typedef variable_rule<PlanetType>::type variable_rule;
        typedef statistic_rule<PlanetType>::type statistic_rule;

        name_token_rule final_token;
        rule constant;
        variable_rule variable;
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
