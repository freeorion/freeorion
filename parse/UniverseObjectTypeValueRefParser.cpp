#include "ValueRefParserImpl.h"

#include "EnumParser.h"


namespace {
    struct universe_object_type_parser_rules {
        universe_object_type_parser_rules()  {
            qi::_1_type _1;
            qi::_a_type _a;
            qi::_val_type _val;
            using phoenix::new_;
            using phoenix::push_back;

            const parse::lexer& tok = parse::lexer::instance();

            variable_name
                %=   tok.ObjectType_
                ;

            constant
                =    parse::enum_parser<UniverseObjectType>() [ _val = new_<ValueRef::Constant<UniverseObjectType> >(_1) ]
                ;

            variable
                =    variable_scope [ push_back(_a, _1) ] > '.'
                >   -(container_type [ push_back(_a, _1) ] > '.')
                >    variable_name [ push_back(_a, _1), _val = new_<ValueRef::Variable<UniverseObjectType> >(_a) ]
                ;


            initialize_nonnumeric_statistic_parser<UniverseObjectType>(statistic, variable_name);

            primary_expr
                %=   constant
                |    variable
                |    statistic
                ;

            variable_name.name("ObjectType variable name (e.g., ObjectType)");
            constant.name("ObjectType");
            variable.name("ObjectType variable");
            statistic.name("ObjectType statistic");
            primary_expr.name("ObjectType expression");

#if DEBUG_VALUEREF_PARSERS
            debug(variable_name);
            debug(constant);
            debug(variable);
            debug(statistic);
            debug(primary_expr);
#endif
        }

        typedef parse::value_ref_parser_rule<UniverseObjectType>::type rule;
        typedef variable_rule<UniverseObjectType>::type variable_rule;
        typedef statistic_rule<UniverseObjectType>::type statistic_rule;

        name_token_rule variable_name;
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
