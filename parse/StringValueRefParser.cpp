#include "ValueRefParserImpl.h"

#include "EnumParser.h"

#include <GG/ReportParseError.h>


namespace {

    struct string_parser_rules
    {
        string_parser_rules()
            {
                qi::_1_type _1;
                qi::_a_type _a;
                qi::_val_type _val;
                qi::as_string_type as_string;
                using phoenix::push_back;
                using phoenix::new_;

                const parse::lexer& tok = parse::lexer::instance();

                final_token
                    %=   tok.Name_
                    |    tok.Species_
                    |    tok.BuildingType_
                    |    tok.Focus_
                    ;

                constant
                    =    tok.string [ _val = new_<ValueRef::Constant<std::string> >(_1) ]
                    |    as_string [
                              parse::enum_parser<PlanetSize>()
                          |   parse::enum_parser<PlanetType>()
                          |   parse::enum_parser<PlanetEnvironment>()
                          |   parse::enum_parser<UniverseObjectType>()
                          |   parse::enum_parser<StarType>()
                          |   tok.double_
                          |   tok.int_
                         ]
                    ;

                variable
                    = (
                           first_token [ push_back(_a, _1) ] > '.'
                       >  -(container_token [ push_back(_a, _1) ] > '.')
                       >   (
                                final_token
                                [ push_back(_a, _1), _val = new_<ValueRef::Variable<std::string> >(_a) ]
                            |   int_var_final_token()
                                [ push_back(_a, _1), _val = new_<ValueRef::StringCast<int> >(new_<ValueRef::Variable<int> >(_a)) ]
                            |   double_var_final_token()
                                [ push_back(_a, _1), _val = new_<ValueRef::StringCast<double> >(new_<ValueRef::Variable<double> >(_a)) ]
                           )
                      )
                    | (
                          tok.CurrentTurn_ [ push_back(_a, _1), _val = new_<ValueRef::StringCast<int> >(new_<ValueRef::Variable<int> >(_a)) ]
                       |  tok.Value_ [ push_back(_a, _1), _val = new_<ValueRef::Variable<std::string> >(_a) ]
                      )
                    ;

                initialize_nonnumeric_statistic_parser<std::string>(statistic, final_token);

                int_statistic
                    =    int_var_statistic() [ _val = new_<ValueRef::StringCast<int> >(_1) ]
                    ;

                double_statistic
                    =    double_var_statistic() [ _val = new_<ValueRef::StringCast<double> >(_1) ]
                    ;

                expr
                    %=   constant
                    |    variable
                    |    int_statistic
                    |    double_statistic
                    |    statistic
                    ;

                final_token.name("string variable name (e.g., Name)");
                constant.name("string");
                variable.name("string variable");
                statistic.name("string statistic");
                int_statistic.name("integer statistic");
                double_statistic.name("real number statistic");
                expr.name("string expression");

#if DEBUG_VALUEREF_PARSERS
                debug(final_token);
                debug(constant);
                debug(variable);
                debug(statistic);
                debug(int_statistic);
                debug(double_statistic);
                debug(expr);
#endif
            }

        typedef parse::value_ref_parser_rule<std::string>::type rule;
        typedef variable_rule<std::string>::type variable_rule;
        typedef statistic_rule<std::string>::type statistic_rule;

        name_token_rule final_token;
        rule constant;
        variable_rule variable;
        statistic_rule statistic;
        rule int_statistic;
        rule double_statistic;
        rule expr;
        rule primary_expr;
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
