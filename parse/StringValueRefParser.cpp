#include "ValueRefParserImpl.h"

#include "EnumParser.h"


namespace { struct string_parser_rules {
        string_parser_rules() {
            qi::_1_type _1;
            qi::_a_type _a;
            qi::_b_type _b;
            qi::_val_type _val;
            qi::as_string_type as_string;
            using phoenix::construct;
            using phoenix::push_back;
            using phoenix::new_;

            const parse::lexer& tok = parse::lexer::instance();

            variable_name
                %=   tok.Name_
                |    tok.Species_
                |    tok.BuildingType_
                |    tok.Focus_
                |    tok.PreferredFocus_
                |    tok.OwnerLeastExpensiveEnqueuedTech_
                |    tok.OwnerMostExpensiveEnqueuedTech_
                |    tok.OwnerMostRPCostLeftEnqueuedTech_
                |    tok.OwnerMostRPSpentEnqueuedTech_
                |    tok.OwnerTopPriorityEnqueuedTech_
                ;

            constant
                =   tok.string [ _val = new_<ValueRef::Constant<std::string> >(_1) ]
                |   as_string [
                            parse::enum_parser<PlanetSize>()
                        |   parse::enum_parser<PlanetType>()
                        |   parse::enum_parser<PlanetEnvironment>()
                        |   parse::enum_parser<UniverseObjectType>()
                        |   parse::enum_parser<StarType>()
                        |   tok.double_
                        |   tok.int_
                    ]
                ;

            free_variable
                =   tok.Value_
                    [ _val = new_<ValueRef::Variable<std::string> >(ValueRef::EFFECT_TARGET_VALUE_REFERENCE) ]
                |   tok.GalaxySeed_
                    [ _val = new_<ValueRef::Variable<std::string> >(ValueRef::NON_OBJECT_REFERENCE, _1) ]
                |   int_free_variable()
                    [ _val = new_<ValueRef::StringCast<int> >(_1) ]
                |   double_free_variable()
                    [ _val = new_<ValueRef::StringCast<double> >(_1) ]
                ;

            bound_variable
                = (
                        variable_scope()  [ _b = _1 ] > '.'
                    >  -(container_type() [ push_back(_a, construct<std::string>(_1)) ] > '.')
                    >>  (
                            variable_name
                            [ push_back(_a, construct<std::string>(_1)),
                              _val = new_<ValueRef::Variable<std::string> >(_b, _a) ]
                        |   int_bound_variable_name()
                            [ push_back(_a, construct<std::string>(_1)),
                              _val = new_<ValueRef::StringCast<int> >(new_<ValueRef::Variable<int> >(_b, _a)) ]
                        |   double_bound_variable_name()
                            [ push_back(_a, construct<std::string>(_1)),
                              _val = new_<ValueRef::StringCast<double> >(new_<ValueRef::Variable<double> >(_b, _a)) ]
                        )
                    )
                ;

            initialize_nonnumeric_statistic_parser<std::string>(statistic, variable_name);

            int_statistic
                =    int_var_statistic() [ _val = new_<ValueRef::StringCast<int> >(_1) ]
                ;

            double_statistic
                =    double_var_statistic() [ _val = new_<ValueRef::StringCast<double> >(_1) ]
                ;

            expr
                %=   constant
                |    free_variable
                |    bound_variable
                |    int_statistic
                |    double_statistic
                |    statistic
                ;

            variable_name.name("string bound_variable name (e.g., Name)");
            constant.name("string");
            free_variable.name("free string variable");
            bound_variable.name("string bound_variable");
            statistic.name("string statistic");
            int_statistic.name("integer statistic");
            double_statistic.name("real number statistic");
            expr.name("string expression");

#if DEBUG_VALUEREF_PARSERS
            debug(variable_name);
            debug(constant);
            debug(free_variable);
            debug(bound_variable);
            debug(statistic);
            debug(int_statistic);
            debug(double_statistic);
            debug(expr);
#endif
        }

        typedef parse::value_ref_parser_rule<std::string>::type rule;
        typedef variable_rule<std::string>::type variable_rule;
        typedef statistic_rule<std::string>::type statistic_rule;

        name_token_rule variable_name;
        rule            constant;
        variable_rule   free_variable;
        variable_rule   bound_variable;
        statistic_rule  statistic;
        rule            int_statistic;
        rule            double_statistic;
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
