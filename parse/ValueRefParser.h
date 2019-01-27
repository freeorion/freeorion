#ifndef _ValueRefParser_h_
#define _ValueRefParser_h_

#include "EnumParser.h"
#include "MovableEnvelope.h"

#include "../universe/ValueRefFwd.h"
#include "../universe/EnumsFwd.h"

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>

namespace Condition {
    struct ConditionBase;
}

namespace parse { namespace detail {
    // TODO: Investigate refactoring ValueRef to use variant,
    // for increased locality of reference.
    template <typename T>
    using value_ref_payload = MovableEnvelope<ValueRef::ValueRefBase<T>>;
    template <typename T>
    using value_ref_signature = value_ref_payload<T> ();
    template <typename T>
    using value_ref_rule = detail::rule<value_ref_signature<T>>;
    template <typename T>
    using value_ref_grammar = detail::grammar<value_ref_signature<T>>;

    using condition_payload        = MovableEnvelope<Condition::ConditionBase>;
    using condition_signature      = condition_payload ();
    using condition_parser_grammar = grammar<condition_signature>;

    using name_token_rule = rule<std::string ()>;
    using reference_token_rule = rule<ValueRef::ReferenceType ()>;
    const parse::detail::reference_token_rule variable_scope(const parse::lexer& tok);
    const parse::detail::name_token_rule container_type(const parse::lexer& tok);


    template <typename T>
    using variable_payload = MovableEnvelope<ValueRef::Variable<T>>;
    template <typename T>
    using variable_signature = variable_payload<T> ();
    template <typename T>
    using variable_rule = rule<
        variable_signature<T>,
        boost::spirit::qi::locals<
            std::vector<std::string>,
            ValueRef::ReferenceType
            >
        >;

    template <typename T>
    struct simple_variable_rules
    {
        simple_variable_rules(const std::string& type_name, const parse::lexer& tok);

        name_token_rule             bound_variable_name;
        name_token_rule             free_variable_name;
        detail::value_ref_rule<T>   constant;
        variable_rule<T>            free_variable;
        variable_rule<T>            bound_variable;
        variable_rule<T>            unwrapped_bound_variable;
        variable_rule<T>            value_wrapped_bound_variable;
        detail::value_ref_rule<T>   simple;
        reference_token_rule        variable_scope_rule;
        name_token_rule             container_type_rule;
    };

    template <typename T>
    using statistic_payload = MovableEnvelope<ValueRef::Statistic<T>>;
    template <typename T>
    using statistic_signature = statistic_payload<T> ();
    template <typename T>
    using statistic_rule = rule<
        statistic_signature<T>,
        boost::spirit::qi::locals<
            value_ref_payload<T>,
            ValueRef::StatisticType
            >
        >;

    template <typename T>
    using expression_rule = rule<
        value_ref_payload<T> (),
        boost::spirit::qi::locals<
            value_ref_payload<T>,
            value_ref_payload<T>,
            ValueRef::OpType,
            std::vector<value_ref_payload<T>>
            >
        >;

    template <typename T>
    struct arithmetic_rules {
        arithmetic_rules(const std::string& type_name,
                         const parse::lexer& tok,
                         Labeller& label,
                         const condition_parser_grammar& condition_parser);

        parse::statistic_enum_grammar   statistic_type_enum;
        expression_rule<T>              functional_expr;
        expression_rule<T>              exponential_expr;
        expression_rule<T>              multiplicative_expr;
        expression_rule<T>              additive_expr;
        detail::value_ref_rule<T>       primary_expr;
        detail::value_ref_rule<T>       statistic_value_ref_expr;
        statistic_rule<T>               statistic_collection_expr;
        statistic_rule<T>               statistic_value_expr;
        statistic_rule<T>               statistic_expr;
        detail::value_ref_rule<T>       expr;
    };

    struct simple_int_parser_rules : public simple_variable_rules<int> {
        simple_int_parser_rules(const parse::lexer& tok);
    };

    struct simple_double_parser_rules : public simple_variable_rules<double> {
        simple_double_parser_rules(const parse::lexer& tok);
    };

    template <typename T>
    using complex_variable_payload = MovableEnvelope<ValueRef::ComplexVariable<T>>;
    template <typename T>
    using complex_variable_signature = complex_variable_payload<T> ();
    template <typename T>
    using complex_variable_rule = rule<complex_variable_signature<T>>;
    template <typename T>
    using complex_variable_grammar = grammar<complex_variable_signature<T>>;

    struct string_complex_parser_grammar : public complex_variable_grammar<std::string> {
        string_complex_parser_grammar(const parse::lexer& tok,
                                      Labeller& label,
                                      const value_ref_grammar<std::string>& string_grammar);

        simple_int_parser_rules             simple_int_rules;
        complex_variable_rule<std::string>  game_rule;
        complex_variable_rule<std::string>  empire_ref;
        complex_variable_rule<std::string>  empire_empire_ref;
        complex_variable_rule<std::string>  start;
    };

    template <typename T>
    void initialize_bound_variable_parser(
        variable_rule<T>& bound_variable,
        variable_rule<T>& unwrapped_bound_variable,
        variable_rule<T>& value_wrapped_bound_variable,
        const name_token_rule& variable_name,
        const reference_token_rule& variable_scope_rule,
        const name_token_rule& container_type_rule,
        const parse::lexer& tok)
    {
        using boost::phoenix::construct;
        using boost::phoenix::new_;
        using boost::phoenix::push_back;

        boost::spirit::qi::_1_type _1;
        boost::spirit::qi::_2_type _2;
        boost::spirit::qi::_3_type _3;
        boost::spirit::qi::lit_type lit;
        boost::spirit::qi::_val_type _val;
        boost::spirit::qi::omit_type omit_;
        const boost::phoenix::function<construct_movable> construct_movable_;
        const boost::phoenix::function<deconstruct_movable> deconstruct_movable_;

        unwrapped_bound_variable
            = (
                     variable_scope_rule >> '.'
                >> -(container_type_rule > '.')
                >>   variable_name
              ) [ _val = construct_movable_(new_<ValueRef::Variable<T>>(
                  _1, construct<boost::optional<std::string>>(_2),
                  construct<std::string>(_3), false)) ];

        value_wrapped_bound_variable
            = (
                omit_[tok.Value_] >> '('
                >>   variable_scope_rule >> '.'
                >> -(container_type_rule > '.')
                >>   variable_name
                >>   ')'
              ) [ _val = construct_movable_(new_<ValueRef::Variable<T>>(
                  _1, construct<boost::optional<std::string>>(_2),
                  construct<std::string>(_3), true)) ];

        bound_variable
            =
                value_wrapped_bound_variable [ _val = _1 ]
            |   unwrapped_bound_variable [ _val = _1 ]
            ;
    }
}}

namespace parse {
    struct string_parser_grammar : public detail::value_ref_grammar<std::string> {
        string_parser_grammar(const lexer& tok,
                              detail::Labeller& label,
                              const detail::condition_parser_grammar& condition_parser);

        detail::string_complex_parser_grammar   string_var_complex;
        detail::name_token_rule                 bound_variable_name;
        detail::value_ref_rule<std::string>     constant;
        detail::value_ref_rule<std::string>     free_variable;
        detail::variable_rule<std::string>      bound_variable;
        detail::variable_rule<std::string>      unwrapped_bound_variable;
        detail::variable_rule<std::string>      value_wrapped_bound_variable;
        detail::value_ref_rule<std::string>     statistic_sub_value_ref;
        detail::statistic_rule<std::string>     statistic;
        detail::expression_rule<std::string>    function_expr;
        detail::expression_rule<std::string>    operated_expr;
        detail::value_ref_rule<std::string>     expr;
        detail::value_ref_rule<std::string>     primary_expr;
        detail::reference_token_rule            variable_scope_rule;
        detail::name_token_rule                 container_type_rule;
    };

    struct int_arithmetic_rules;

    struct int_complex_parser_grammar : public detail::complex_variable_grammar<int> {
        int_complex_parser_grammar(const lexer& tok,
                                   detail::Labeller& label,
                                   const int_arithmetic_rules& _int_arith_rules,
                                   const detail::value_ref_grammar<std::string>& string_grammar);

        const int_arithmetic_rules&         int_rules;
        ship_part_class_enum_grammar        ship_part_class_enum;
        detail::complex_variable_rule<int>  game_rule;
        detail::complex_variable_rule<int>  empire_name_ref;
        detail::complex_variable_rule<int>  empire_ships_destroyed;
        detail::complex_variable_rule<int>  jumps_between;
        //complex_variable_rule<int>          jumps_between_by_empire_supply;
        detail::complex_variable_rule<int>  outposts_owned;
        detail::complex_variable_rule<int>  parts_in_ship_design;
        detail::complex_variable_rule<int>  part_class_in_ship_design;
        detail::value_ref_rule<int>         part_class_as_int;
        detail::complex_variable_rule<int>  ship_parts_owned;
        detail::complex_variable_rule<int>  empire_design_ref;
        detail::complex_variable_rule<int>  slots_in_hull;
        detail::complex_variable_rule<int>  slots_in_ship_design;
        detail::complex_variable_rule<int>  special_added_on_turn;
        detail::complex_variable_rule<int>  start;
    };

    struct int_arithmetic_rules : public detail::arithmetic_rules<int> {
        int_arithmetic_rules(
            const lexer& tok,
            detail::Labeller& label,
            const detail::condition_parser_grammar& condition_parser,
            const detail::value_ref_grammar<std::string>& string_grammar);
        detail::simple_int_parser_rules  simple_int_rules;
        int_complex_parser_grammar int_complex_grammar;
    };

    struct double_complex_parser_grammar : public detail::complex_variable_grammar<double> {
        double_complex_parser_grammar(const lexer& tok,
                                      detail::Labeller& label,
                                      const detail::condition_parser_grammar& condition_parser,
                                      const detail::value_ref_grammar<std::string>& string_grammar);

        detail::simple_int_parser_rules         simple_int_rules;
        detail::complex_variable_rule<double>   name_property_rule;
        detail::complex_variable_rule<double>   empire_meter_value;
        detail::complex_variable_rule<double>   direct_distance;
        detail::complex_variable_rule<double>   shortest_path;
        detail::rule<detail::value_ref_payload<std::string>()> species_opinion;
        detail::complex_variable_rule<double>   species_empire_opinion;
        detail::complex_variable_rule<double>   species_species_opinion;
        detail::complex_variable_rule<double>   special_capacity;
        detail::complex_variable_rule<double>   part_meter;
        detail::complex_variable_rule<double>   start;
        parse::ship_part_meter_enum_grammar     ship_part_meter_type_enum;
    };

    struct double_parser_rules : public detail::arithmetic_rules<double> {
        double_parser_rules(
            const lexer& tok,
            detail::Labeller& label,
            const detail::condition_parser_grammar& condition_parser,
            const detail::value_ref_grammar<std::string>& string_grammar);

        int_arithmetic_rules                int_rules;
        detail::simple_int_parser_rules     simple_int_rules;
        detail::simple_double_parser_rules  simple_double_rules;
        int_complex_parser_grammar          int_complex_grammar;
        double_complex_parser_grammar       double_complex_grammar;
        detail::value_ref_rule<double>      int_constant_cast;
        detail::value_ref_rule<double>      int_bound_variable_cast;
        detail::value_ref_rule<double>      int_free_variable_cast;
        detail::value_ref_rule<double>      int_statistic_cast;
        detail::value_ref_rule<double>      int_complex_variable_cast;
    };

    struct castable_as_int_parser_rules {
        castable_as_int_parser_rules(const lexer& tok,
                                     detail::Labeller& label,
                                     const detail::condition_parser_grammar& condition_parser,
                                     const detail::value_ref_grammar<std::string>& string_grammar);

        int_arithmetic_rules        int_rules;
        double_parser_rules         double_rules;
        detail::value_ref_rule<int> castable_expr;
        detail::value_ref_rule<int> flexible_int;
    };
}

#endif // _ValueRefParser_h_
