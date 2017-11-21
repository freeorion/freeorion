#include "EnumValueRefRules.h"

#include "MovableEnvelope.h"
#include "../universe/ValueRef.h"

#include <boost/spirit/include/phoenix.hpp>

namespace parse { namespace detail {

    template <typename T>
    void initialize_nonnumeric_statistic_parser(
        statistic_rule<T>& statistic,
        const parse::lexer& tok,
        Labeller& labeller,
        const condition_parser_grammar& condition_parser,
        const value_ref_rule<T>& value_ref)
    {
        using boost::phoenix::construct;
        using boost::phoenix::new_;
        using boost::phoenix::push_back;

        boost::spirit::qi::_1_type _1;
        boost::spirit::qi::_a_type _a;
        boost::spirit::qi::_b_type _b;
        boost::spirit::qi::_val_type _val;
        boost::spirit::qi::_pass_type _pass;
        const boost::phoenix::function<construct_movable> construct_movable_;
        const boost::phoenix::function<deconstruct_movable> deconstruct_movable_;

        statistic
            =   (tok.Statistic_ >>  tok.Mode_ [ _b = ValueRef::MODE ])
            >   labeller.rule(Value_token)     >     value_ref [ _a = _1 ]
            >   labeller.rule(Condition_token) >     condition_parser
            [ _val = construct_movable_(new_<ValueRef::Statistic<T>>(deconstruct_movable_(_a, _pass), _b, deconstruct_movable_(_1, _pass))) ]
            ;
    }

    template void initialize_nonnumeric_statistic_parser<std::string>(
        statistic_rule<std::string>& statistic,
        const parse::lexer& tok,
        Labeller& labeller,
        const condition_parser_grammar& condition_parser,
        const value_ref_rule<std::string>& value_ref);

    template <typename T>
    enum_value_ref_rules<T>::enum_value_ref_rules(
        const std::string& type_name,
        const parse::lexer& tok,
        Labeller& labeller,
        const condition_parser_grammar& condition_parser)
    {
        using boost::phoenix::new_;
        using boost::phoenix::push_back;

        boost::spirit::qi::_1_type _1;
        boost::spirit::qi::_2_type _2;
        boost::spirit::qi::_val_type _val;
        boost::spirit::qi::_pass_type _pass;
        boost::spirit::qi::lit_type lit;
        const boost::phoenix::function<construct_movable> construct_movable_;
        const boost::phoenix::function<deconstruct_movable> deconstruct_movable_;
        const boost::phoenix::function<deconstruct_movable_vector> deconstruct_movable_vector_;

        constant_expr
            =   enum_expr [ _val = construct_movable_(new_<ValueRef::Constant<T>>(_1)) ]
            ;

        variable_scope_rule = variable_scope(tok);
        container_type_rule = container_type(tok);
        initialize_bound_variable_parser<T>(bound_variable_expr, variable_name, variable_scope_rule, container_type_rule, tok);

        statistic_sub_value_ref
            =   constant_expr
            |   bound_variable_expr
            |   free_variable_expr
            |   complex_expr
            ;

        selection_operator
            =   tok.OneOf_  [ _val = ValueRef::RANDOM_PICK ]
            |   tok.Min_    [ _val = ValueRef::MINIMUM ]
            |   tok.Max_    [ _val = ValueRef::MAXIMUM ];

        selection_expr
            = (selection_operator > '(' > (expr % ',') > ')')
            [ _val = construct_movable_(new_<ValueRef::Operation<T>>(_1, deconstruct_movable_vector_(_2, _pass))) ];

        functional_expr %=  selection_expr | primary_expr;

        expr
            =   functional_expr
            ;

        initialize_nonnumeric_statistic_parser<T>(
            statistic_expr, tok, labeller, condition_parser, statistic_sub_value_ref);

        primary_expr
            =   constant_expr
            |   bound_variable_expr
            |   free_variable_expr
            |   statistic_expr
            |   complex_expr
            ;

#if DEBUG_VALUEREF_PARSERS
        debug(variable_name);
        debug(enum_expr);
        debug(constant_expr);
        debug(free_variable_expr);
        debug(bound_variable_expr);
        debug(statistic_value_ref_expr);
        debug(statistic_expr);
        debug(functional_expr);
        debug(primary_expr);
        debug(expr);
#endif

        variable_name.name(type_name + " variable name");
        enum_expr.name(type_name);
        constant_expr.name(type_name + " constant");
        free_variable_expr.name(type_name + " free variable");
        bound_variable_expr.name(type_name + " variable");
        statistic_sub_value_ref.name(type_name + " statistic subvalue");
        statistic_expr.name(type_name + " statistic");
        primary_expr.name(type_name + " expression");
        expr.name(type_name + " expression");
    }


    // Explicit instantiation to prevent costly recompilation in multiple units
    template enum_value_ref_rules<PlanetEnvironment>::enum_value_ref_rules(
        const std::string& type_name, const parse::lexer& tok,
        Labeller& labeller, const condition_parser_grammar& condition_parser);
    template enum_value_ref_rules<PlanetSize>::enum_value_ref_rules(
        const std::string& type_name, const parse::lexer& tok,
        Labeller& labeller, const condition_parser_grammar& condition_parser);
    template enum_value_ref_rules<PlanetType>::enum_value_ref_rules(
        const std::string& type_name, const parse::lexer& tok,
        Labeller& labeller, const condition_parser_grammar& condition_parser);
    template enum_value_ref_rules<StarType>::enum_value_ref_rules(
        const std::string& type_name, const parse::lexer& tok,
        Labeller& labeller, const condition_parser_grammar& condition_parser);
    template enum_value_ref_rules<UniverseObjectType>::enum_value_ref_rules(
        const std::string& type_name, const parse::lexer& tok,
        Labeller& labeller, const condition_parser_grammar& condition_parser);
    template enum_value_ref_rules<Visibility>::enum_value_ref_rules(
        const std::string& type_name, const parse::lexer& tok,
        Labeller& labeller, const condition_parser_grammar& condition_parser);

    }}
