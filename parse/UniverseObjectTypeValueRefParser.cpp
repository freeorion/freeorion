#include "ValueRefParserImpl.h"

#include "EnumParser.h"

#include <GG/ReportParseError.h>


namespace {

    struct universe_object_type_parser_rules
    {
        universe_object_type_parser_rules()
            {
                qi::_1_type _1;
                qi::_a_type _a;
                qi::_val_type _val;
                using phoenix::new_;
                using phoenix::push_back;
                using phoenix::static_cast_;

                const parse::lexer& tok = parse::lexer::instance();

                final_token
                    %=   tok.ObjectType_
                    ;

                constant
                    =    parse::enum_parser<UniverseObjectType>() [ _val = new_<ValueRef::Constant<UniverseObjectType> >(_1) ]
                    ;

                variable
                    =    first_token [ push_back(_a, _1) ] > '.'
                    >   -(container_token [ push_back(_a, _1) ] > '.')
                    >    final_token [ push_back(_a, _1), _val = new_<ValueRef::Variable<UniverseObjectType> >(_a) ]
                    ;


                initialize_nonnumeric_statistic_parser<UniverseObjectType>(statistic, final_token);

                primary_expr
                    %=   constant
                    |    variable
                    |    statistic
                    ;

                final_token.name("ObjectType variable name (e.g., ObjectType)");
                constant.name("ObjectType");
                variable.name("ObjectType variable");
                statistic.name("ObjectType statistic");
                primary_expr.name("ObjectType expression");

#if DEBUG_VALUEREF_PARSERS
                debug(final_token);
                debug(constant);
                debug(variable);
                debug(statistic);
                debug(primary_expr);
#endif
            }

        typedef parse::value_ref_parser_rule<UniverseObjectType>::type rule;
        typedef variable_rule<UniverseObjectType>::type variable_rule;
        typedef statistic_rule<UniverseObjectType>::type statistic_rule;

        name_token_rule final_token;
        rule constant;
        variable_rule variable;
        statistic_rule statistic;
        rule primary_expr;
    };

}


namespace parse {

    template <>
    value_ref_parser_rule<UniverseObjectType>::type& value_ref_parser<UniverseObjectType>()
    {
        static universe_object_type_parser_rules retval;
        return retval.primary_expr;
    }

}
