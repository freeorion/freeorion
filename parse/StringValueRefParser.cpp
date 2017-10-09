#include "ValueRefParserImpl.h"


namespace parse {
    string_parser_grammar::string_parser_grammar(
        const parse::lexer& tok,
        parse::detail::Labeller& labeller,
        const detail::condition_parser_grammar& condition_parser
    ) :
        string_parser_grammar::base_type(expr, "string_parser_grammar"),
        string_var_complex(tok, labeller, *this)
    {
        namespace phoenix = boost::phoenix;
        namespace qi = boost::spirit::qi;

        using phoenix::new_;
        using phoenix::push_back;

        qi::_1_type _1;
        qi::_a_type _a;
        qi::_b_type _b;
        qi::_c_type _c;
        qi::_d_type _d;
        qi::_val_type _val;
        qi::lit_type lit;

        const std::string TOK_CURRENT_CONTENT{"CurrentContent"};

        bound_variable_name
            %=   tok.Name_
            |    tok.Species_
            |    tok.BuildingType_
            |    tok.Focus_
            |    tok.PreferredFocus_
            ;

        constant
            =   tok.string          [ _val = new_<ValueRef::Constant<std::string>>(_1) ]
            |  (    tok.CurrentContent_
                    |   tok.ThisBuilding_
                    |   tok.ThisField_
                    |   tok.ThisHull_
                    |   tok.ThisPart_   // various aliases for this reference in scripts, allowing scripter to use their preference
                    |   tok.ThisTech_
                    |   tok.ThisSpecies_
                    |   tok.ThisSpecial_
               ) [ _val = new_<ValueRef::Constant<std::string>>(TOK_CURRENT_CONTENT) ]
            ;

        free_variable
            =   tok.Value_          [ _val = new_<ValueRef::Variable<std::string>>(ValueRef::EFFECT_TARGET_VALUE_REFERENCE) ]
            |   tok.GalaxySeed_     [ _val = new_<ValueRef::Variable<std::string>>(ValueRef::NON_OBJECT_REFERENCE, _1) ]
            ;

        variable_scope_rule = variable_scope(tok);
        container_type_rule = container_type(tok);
        initialize_bound_variable_parser<std::string>(bound_variable, bound_variable_name,
                                                      variable_scope_rule, container_type_rule);

        statistic_sub_value_ref
            =   constant
            |   bound_variable
            |   free_variable
            |   string_var_complex
            ;

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
                    [ _val = new_<ValueRef::Operation<std::string>>(_c, _d) ] >   ')'
                )
                |   (
                    tok.UserString_ >   '(' >   expr[ _val = new_<ValueRef::UserStringLookup<std::string>>(_1) ] >   ')'
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
                    function_expr [ _a = _1 ]
                    >>  lit('+') [ _c = ValueRef::PLUS ]
                    >>  function_expr [ _b = new_<ValueRef::Operation<std::string>>(_c, _a, _1) ]
                    [ _val = _b ]
                )
                |   (
                    function_expr [ push_back(_d, _1) ]     // template string
                    >>+('%' >   function_expr [ push_back(_d, _1) ] )   // must have at least one sub-string
                    [ _val = new_<ValueRef::Operation<std::string>>(ValueRef::SUBSTITUTION, _d) ]
                )
                |   (
                    function_expr [ _val = _1 ]
                )
            )
            ;

        expr
            =   operated_expr
            ;

        initialize_nonnumeric_statistic_parser<std::string>(statistic, tok, condition_parser, statistic_sub_value_ref);

        primary_expr
            =   constant
            |   free_variable
            |   bound_variable
            |   statistic
            |   string_var_complex
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

    parse::detail::name_token_rule bound_variable_name;
    parse::value_ref_rule<std::string> constant;
    parse::value_ref_rule<std::string> free_variable;
    parse::detail::variable_rule<std::string> bound_variable;
    parse::value_ref_rule<std::string> statistic_sub_value_ref;
    parse::detail::statistic_rule<std::string> statistic;
    parse::detail::expression_rule<std::string> function_expr;
    parse::detail::expression_rule<std::string> operated_expr;
    parse::value_ref_rule<std::string> expr;
    parse::value_ref_rule<std::string> primary_expr;
    parse::detail::reference_token_rule variable_scope_rule;
    parse::detail::name_token_rule container_type_rule;
};
