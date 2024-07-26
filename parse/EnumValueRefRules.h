#ifndef _EnumValueRefRules_h_
#define _EnumValueRefRules_h_

#include "ValueRefParser.h"
#include "MovableEnvelope.h"

namespace parse::detail {

    template <typename T>
    void initialize_nonnumeric_statistic_parser(
        parse::detail::statistic_rule<T>& statistic,
        const parse::lexer& tok,
        parse::detail::Labeller& label,
        const parse::detail::condition_parser_grammar& condition_parser,
        const typename parse::detail::value_ref_rule<T>& value_ref);

    template <typename T>
    struct enum_value_ref_rules {
        enum_value_ref_rules(const std::string& type_name,
                             const lexer& tok,
                             Labeller& label,
                             const condition_parser_grammar& condition_parser);

        rule<ValueRef::OpType ()> selection_operator;
        value_ref_rule<T> selection_expr;
        name_token_rule variable_name;
        enum_rule<T> enum_expr;
        value_ref_rule<T> constant_expr;
        value_ref_rule<T> free_variable_expr;
        variable_rule<T> bound_variable_expr;
        variable_rule<T> unwrapped_bound_variable_expr;
        variable_rule<T> value_wrapped_bound_variable_expr;
        expression_rule<T> functional_expr;
        value_ref_rule<T> named_lookup_expr;
        value_ref_rule<T> primary_expr;
        value_ref_rule<T> statistic_sub_value_ref;
        statistic_rule<T> statistic_expr;
        complex_variable_rule<T> complex_expr;
        value_ref_rule<T> expr;
        reference_token_rule variable_scope_rule;
        container_token_rule container_type_rule;
    };

    template <typename T>
    void initialize_nonnumeric_statistic_parser(
        statistic_rule<T>& statistic,
        const parse::lexer& tok,
        Labeller& label,
        const condition_parser_grammar& condition_parser,
        const value_ref_rule<T>& value_ref)
    {
        using boost::phoenix::construct;
        using boost::phoenix::new_;
        using boost::phoenix::push_back;

        boost::spirit::qi::_1_type _1;
        boost::spirit::qi::_2_type _2;
        boost::spirit::qi::_val_type _val;
        boost::spirit::qi::_pass_type _pass;
        boost::spirit::qi::omit_type omit_;
        const boost::phoenix::function<construct_movable> construct_movable_;
        const boost::phoenix::function<deconstruct_movable> deconstruct_movable_;

        statistic
            =  (    (omit_[tok.Statistic_] >>  omit_[tok.Mode_])
                 >   label(tok.value_)     >     value_ref
                 >   label(tok.condition_) >     condition_parser)
            [ _val = construct_movable_(new_<ValueRef::Statistic<T>>(
                deconstruct_movable_(_1, _pass),
                ValueRef::StatisticType::MODE,
                deconstruct_movable_(_2, _pass))) ]
            ;
    }

    template <typename T>
    enum_value_ref_rules<T>::enum_value_ref_rules(
        const std::string& type_name,
        const parse::lexer& tok,
        Labeller& label,
        const condition_parser_grammar& condition_parser)
    {
        using boost::phoenix::new_;
        using boost::phoenix::push_back;

        boost::spirit::qi::_1_type _1;
        boost::spirit::qi::_2_type _2;
        boost::spirit::qi::_4_type _4;
        boost::spirit::qi::_val_type _val;
        boost::spirit::qi::_pass_type _pass;
        boost::spirit::qi::lit_type lit;
        const boost::phoenix::function<construct_movable> construct_movable_;
        //const boost::phoenix::function<deconstruct_movable> deconstruct_movable_;
        const boost::phoenix::function<deconstruct_movable_vector> deconstruct_movable_vector_;

        constant_expr
            =   enum_expr [ _val = construct_movable_(new_<ValueRef::Constant<T>>(_1)) ]
            ;

        variable_scope_rule = variable_scope(tok);
        container_type_rule = container(tok);
        initialize_bound_variable_parser<T>(
            bound_variable_expr, unwrapped_bound_variable_expr,
            value_wrapped_bound_variable_expr, variable_name,
            variable_scope_rule, container_type_rule, tok);

        statistic_sub_value_ref
            =   constant_expr
            |   bound_variable_expr
            |   free_variable_expr
            |   complex_expr
            ;

        selection_operator
            =   tok.OneOf_  [ _val = ValueRef::OpType::RANDOM_PICK ]
            |   tok.Min_    [ _val = ValueRef::OpType::MINIMUM ]
            |   tok.Max_    [ _val = ValueRef::OpType::MAXIMUM ];

        selection_expr
            = (selection_operator > '(' > (expr % ',') > ')')
            [ _val = construct_movable_(new_<ValueRef::Operation<T>>(_1, deconstruct_movable_vector_(_2, _pass))) ];

        named_lookup_expr
          =   (
                   tok.Named_ >> tok.Value_ >> tok.Lookup_
                >> label(tok.name_)
                >> tok.string
              ) [ _val = construct_movable_(new_<ValueRef::NamedRef<T>>(_4)) ]
            ;

        functional_expr %=  selection_expr | primary_expr;

        expr
            =   functional_expr
            ;

        initialize_nonnumeric_statistic_parser<T>(
            statistic_expr, tok, label, condition_parser, statistic_sub_value_ref);

        primary_expr
            =   constant_expr
            |   bound_variable_expr
            |   free_variable_expr
            |   statistic_expr
            |   complex_expr
            |   named_lookup_expr
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
        debug(named_lookup_expr);
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
        named_lookup_expr.name(type_name + " named valueref");
        primary_expr.name(type_name + " expression");
        expr.name(type_name + " expression");
    }

    /* The following parsers are defined in separate compilation units to
       avoid MSVC running out of memory and throwing:
       fatal error C1060: compiler is out of heap space
    */
    struct planet_environment_parser_rules :
        public enum_value_ref_rules<PlanetEnvironment>
    {
        planet_environment_parser_rules(const lexer& tok,
                                        Labeller& label,
                                        const condition_parser_grammar& condition_parser);
    };

    struct planet_size_parser_rules :
        public enum_value_ref_rules<PlanetSize>
    {
        planet_size_parser_rules(const lexer& tok,
                                 Labeller& label,
                                 const condition_parser_grammar& condition_parser);
    };

    struct planet_type_parser_rules :
        public enum_value_ref_rules<PlanetType>
    {
        planet_type_parser_rules(const lexer& tok,
                                 Labeller& label,
                                 const condition_parser_grammar& condition_parser);
    };

    struct star_type_parser_rules :
        public enum_value_ref_rules<StarType>
    {
        star_type_parser_rules(const lexer& tok,
                               Labeller& label,
                               const condition_parser_grammar& condition_parser);
    };

    struct visibility_complex_parser_grammar : public complex_variable_grammar<Visibility> {
        visibility_complex_parser_grammar(const lexer& tok, Labeller& label);

        simple_int_parser_rules           simple_int_rules;
        complex_variable_rule<Visibility> empire_object_visibility;
        complex_variable_rule<Visibility> start;
    };

    struct visibility_parser_rules :
        public enum_value_ref_rules<Visibility>
    {
        visibility_parser_rules(const lexer& tok,
                                Labeller& label,
                                const condition_parser_grammar& condition_parser);

        visibility_complex_parser_grammar visibility_var_complex_grammar;
    };

    struct universe_object_type_parser_rules :
        public enum_value_ref_rules<UniverseObjectType>
    {
        universe_object_type_parser_rules(const lexer& tok,
                                          Labeller& label,
                                          const condition_parser_grammar& condition_parser);
    };

}


#endif
