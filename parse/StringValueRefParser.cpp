#include "ValueRefParserImpl.h"

#include "EnumParser.h"

namespace { struct string_parser_rules {
        string_parser_rules() {
            qi::_1_type _1;
            qi::_val_type _val;
            using phoenix::new_;

            const parse::lexer& tok = parse::lexer::instance();

            bound_variable_name
                %=   tok.Name_
                |    tok.Species_
                |    tok.BuildingType_
                |    tok.Focus_
                |    tok.PreferredFocus_
                ;

            constant
                =   tok.string          [ _val = new_<ValueRef::Constant<std::string> >(_1) ]
                |   tok.CurrentContent_ [ _val = new_<ValueRef::Constant<std::string> >(_1) ]
                ;

            free_variable
                =   tok.Value_          [ _val = new_<ValueRef::Variable<std::string> >(ValueRef::EFFECT_TARGET_VALUE_REFERENCE) ]
                |   tok.GalaxySeed_     [ _val = new_<ValueRef::Variable<std::string> >(ValueRef::NON_OBJECT_REFERENCE, _1) ]
                ;

            initialize_bound_variable_parser<std::string>(bound_variable, bound_variable_name);

            statistic_sub_value_ref
                =   constant
                |   bound_variable
                |   free_variable
                |   string_var_complex()
                ;

            initialize_nonnumeric_statistic_parser<std::string>(statistic, statistic_sub_value_ref);

            expr
                =   constant
                |   free_variable
                |   bound_variable
                |   statistic
                |   string_var_complex()
                ;

            bound_variable_name.name("string bound_variable name (e.g., Name)");
            constant.name("string");
            free_variable.name("free string variable");
            bound_variable.name("string bound_variable");
            statistic.name("string statistic");
            expr.name("string expression");

#if DEBUG_VALUEREF_PARSERS
            debug(bound_variable_name);
            debug(constant);
            debug(free_variable);
            debug(bound_variable);
            debug(statistic);
            debug(expr);
#endif
        }

        typedef parse::value_ref_parser_rule<std::string>::type rule;
        typedef variable_rule<std::string>::type                variable_rule;
        typedef statistic_rule<std::string>::type               statistic_rule;

        name_token_rule bound_variable_name;
        rule            constant;
        rule            free_variable;
        variable_rule   bound_variable;
        rule            statistic_sub_value_ref;
        statistic_rule  statistic;
        rule            expr;
        rule            primary_expr;
    };
}


namespace parse {
    template <>
    value_ref_parser_rule<std::string>::type& value_ref_parser<std::string>()
    {
        static string_parser_rules retval;
        return retval.expr;
    }
}
