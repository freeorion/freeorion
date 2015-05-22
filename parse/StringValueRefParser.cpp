#include "ValueRefParserImpl.h"

#include "EnumParser.h"

template <>
void initialize_nonnumeric_expression_parsers<std::string>(
    typename expression_rule<std::string>::type&                function_expr,
    typename expression_rule<std::string>::type&                operated_expr,
    typename parse::value_ref_parser_rule<std::string>::type&   expr,
    typename parse::value_ref_parser_rule<std::string>::type&   primary_expr)
{
    qi::_1_type _1;
    qi::_a_type _a;
    qi::_b_type _b;
    qi::_c_type _c;
    qi::_d_type _d;
    qi::_val_type _val;
    qi::lit_type lit;
    using phoenix::new_;
    using phoenix::push_back;

    const parse::lexer& tok = parse::lexer::instance();

    function_expr
        =   (
                (
                    (
                        tok.OneOf_      [ _c = ValueRef::RANDOM_PICK ]
                    |   tok.Min_        [ _c = ValueRef::MINIMUM ]
                    |   tok.Max_        [ _c = ValueRef::MAXIMUM ]
                    )
                    >  '('  >   expr [ push_back(_d, _1) ]
                    >*(','  >   expr [ push_back(_d, _1) ] )
                    [ _val = new_<ValueRef::Operation<std::string> >(_c, _d) ] >   ')'
                )
            |   (
                    primary_expr [ _val = _1 ]
                )
            )
        ;

    operated_expr
        =
            (
                (
                    (
                        function_expr [ _a = _1 ]
                    >>  (
                            (
                                (
                                    lit('+') [ _c = ValueRef::PLUS ]
                                    // intentionally left blank
                                )
                            >>   function_expr [ _b = new_<ValueRef::Operation<std::string> >(_c, _a, _1) ]
                            ) [ _a = _b ]
                        )
                    )
                    [ _val = _a ]
                )
            |   (
                    function_expr [ _val = _1 ]
                )
            )
        ;

    expr
        =   operated_expr
        ;
}

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
                |  (    tok.CurrentContent_
                    |   tok.ThisBuilding_
                    |   tok.ThisField_
                    |   tok.ThisHull_
                    |   tok.ThisPart_   // various aliases for this reference in scripts, allowing scripter to use their preference
                    |   tok.ThisTech_
                    |   tok.ThisSpecies_
                    |   tok.ThisSpecial_
                   ) [ _val = new_<ValueRef::Constant<std::string> >("CurrentContent") ]
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

            initialize_nonnumeric_expression_parsers<std::string>(function_expr, operated_expr, expr, primary_expr);

            initialize_nonnumeric_statistic_parser<std::string>(statistic, statistic_sub_value_ref);

            primary_expr
                =   constant
                |   free_variable
                |   bound_variable
                |   statistic
                |   string_var_complex()
                ;

            bound_variable_name.name("string bound_variable name (e.g., Name)");
            constant.name("quoted string or CurrentContent");
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
        typedef expression_rule<std::string>::type              expression_rule;

        name_token_rule bound_variable_name;
        rule            constant;
        rule            free_variable;
        variable_rule   bound_variable;
        rule            statistic_sub_value_ref;
        statistic_rule  statistic;
        expression_rule function_expr;
        expression_rule operated_expr;
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
