#ifndef _ValueRefParser_h_
#define _ValueRefParser_h_

#include "EnumParser.h"
#include "MovableEnvelope.h"

#include "../universe/ValueRefs.h"
#include "../universe/NamedValueRefManager.h"
#include "../universe/EnumsFwd.h"

#include <boost/spirit/include/qi.hpp>
#include <boost/phoenix.hpp>

namespace Condition {
    struct Condition;
}

namespace parse::detail {
    // TODO: Investigate refactoring ValueRef to use variant,
    // for increased locality of reference.
    template <typename T>
    using value_ref_payload = MovableEnvelope<ValueRef::ValueRef<T>>;
    template <typename T>
    using value_ref_signature = value_ref_payload<T> ();
    template <typename T>
    using value_ref_rule = detail::rule<value_ref_signature<T>>;
    template <typename T>
    using value_ref_grammar = detail::grammar<value_ref_signature<T>>;

    using condition_payload        = MovableEnvelope<Condition::Condition>;
    using condition_signature      = condition_payload ();
    using condition_parser_grammar = grammar<condition_signature>;

    using name_token_rule = rule<std::string ()>;
    using reference_token_rule = rule<ValueRef::ReferenceType ()>;
    using container_token_rule = rule<ValueRef::ContainerType ()>;
    const parse::detail::reference_token_rule variable_scope(const parse::lexer& tok);
    const parse::detail::container_token_rule container(const parse::lexer& tok);


    template <typename T>
    using variable_payload = MovableEnvelope<ValueRef::Variable<T>>;
    template <typename T>
    using variable_signature = variable_payload<T> ();
    template <typename T>
    using variable_rule = rule<
        variable_signature<T>,
        boost::spirit::qi::locals<
            std::string,
            ValueRef::ReferenceType,
            ValueRef::ContainerType
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
        container_token_rule        container_type_rule;
    };

    template <typename T>
    using statistic_payload = MovableEnvelope<ValueRef::ValueRef<T>>;
    template <typename T>
    using statistic_signature = statistic_payload<T> ();
    template <typename T>
    using statistic_rule = rule<
        statistic_signature<T>,
        boost::spirit::qi::locals<
            value_ref_payload<T>,
            ValueRef::StatisticType,
            value_ref_payload<std::string>
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
                         const condition_parser_grammar& condition_parser,
                         const value_ref_grammar<std::string>& string_grammar);

        parse::statistic_enum_grammar   statistic_type_enum;
        expression_rule<T>              functional_expr;
        expression_rule<T>              exponential_expr;
        expression_rule<T>              multiplicative_expr;
        expression_rule<T>              additive_expr;
        detail::value_ref_rule<T>       named_lookup_expr;
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
        const container_token_rule& container_type_rule,
        const parse::lexer& tok)
    {
        using boost::phoenix::construct;
        using boost::phoenix::new_;

        boost::spirit::qi::_1_type _1{};
        boost::spirit::qi::_a_type _a{}; // string
        boost::spirit::qi::_b_type _b{}; // ReferenceType
        boost::spirit::qi::_c_type _c{}; // ContainerType
        boost::spirit::qi::_val_type _val{};
        boost::spirit::qi::omit_type omit_{};
        boost::spirit::qi::eps_type eps{};
        const boost::phoenix::function<construct_movable> construct_movable_{};

        unwrapped_bound_variable
            = (
                     variable_scope_rule [ _b = _1 ] >> '.'
                >>  (
                       (container_type_rule [ _c = _1 ] >> '.') |
                        eps [ _c = ValueRef::ContainerType::NONE ]
                    )
                >>  (variable_name [ _a = construct<std::string>(_1) ])
              ) [ _val = construct_movable_(new_<ValueRef::Variable<T>>(
                  _b, _a, _c, ValueRef::ValueToReturn::Initial)) ];

        value_wrapped_bound_variable
            = (
                omit_[tok.Value_] >> '('
                >>   variable_scope_rule [ _b = _1 ] >> '.'
                >>  (
                       (container_type_rule [ _c = _1 ] >> '.') |
                        eps [ _c = ValueRef::ContainerType::NONE ]
                    )
                >>  (variable_name [ _a = construct<std::string>(_1) ]) > ')'
              ) [ _val = construct_movable_(new_<ValueRef::Variable<T>>(
                  _b, _a, _c, ValueRef::ValueToReturn::Immediate)) ];

        bound_variable
            =
                value_wrapped_bound_variable [ _val = _1 ]
            |   unwrapped_bound_variable [ _val = _1 ]
            ;
    }

    template <typename T>
    void open_and_register_as_string(const std::string& nameref,
                                     ::parse::detail::MovableEnvelope<ValueRef::ValueRef<T>>& obj,
                                     bool& pass)
    {
        if (obj.IsEmptiedEnvelope()) {
            ErrorLogger() << "The parser attempted to extract the unique_ptr from a MovableEnvelope more than once "
                          << "- while looking at a valueref envelope for use in ValueRef registration ";
            pass = false;
        } else {
            ::RegisterValueRef<T>(nameref, obj.OpenEnvelope(pass));
        }
    }

    BOOST_PHOENIX_ADAPT_FUNCTION(void, open_and_register_as_string_, open_and_register_as_string, 3)
}

#include "EnumValueRefRules.h"

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
        detail::value_ref_rule<std::string>     named_string_valueref;
        detail::value_ref_rule<std::string>     named_lookup_expr;
        detail::expression_rule<std::string>    function_expr;
        detail::expression_rule<std::string>    operated_expr;
        detail::value_ref_rule<std::string>     expr;
        detail::value_ref_rule<std::string>     primary_expr;
        detail::reference_token_rule            variable_scope_rule;
        detail::container_token_rule            container_type_rule;
    };

    struct int_arithmetic_rules;

    struct int_complex_parser_grammar : public detail::complex_variable_grammar<int> {
        int_complex_parser_grammar(const lexer& tok,
                                   detail::Labeller& label,
                                   const int_arithmetic_rules& _int_arith_rules,
                                   const detail::condition_parser_grammar& condition_parser,
                                   const detail::value_ref_grammar<std::string>& string_grammar);

        // imported grammars directly used in int_complex_parser_grammar
        const int_arithmetic_rules&         int_rules;
        ship_part_class_enum_grammar        ship_part_class_enum;
        detail::planet_type_parser_rules    planet_type_rules;

        // grammars defined by int_complex_parser_grammar
        detail::complex_variable_rule<int>  game_rule;
        detail::complex_variable_rule<int>  empire_name_ref;
        detail::complex_variable_rule<int>  empire_id_ref;
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
        detail::complex_variable_rule<int>  planet_type_difference;
        detail::complex_variable_rule<int>  start;
    };

    struct int_arithmetic_rules : public detail::arithmetic_rules<int> {
        int_arithmetic_rules(
            const lexer& tok,
            detail::Labeller& label,
            const detail::condition_parser_grammar& condition_parser,
            const detail::value_ref_grammar<std::string>& string_grammar);
        detail::simple_int_parser_rules simple_int_rules;
        int_complex_parser_grammar      int_complex_grammar;
        detail::value_ref_rule<int>     named_int_valueref;
        detail::value_ref_rule<int>     total_fighter_shots;
    };

    struct double_complex_parser_grammar : public detail::complex_variable_grammar<double> {
        double_complex_parser_grammar(const lexer& tok,
                                      detail::Labeller& label,
                                      const detail::condition_parser_grammar& condition_parser,
                                      const detail::value_ref_grammar<std::string>& string_grammar);

        detail::simple_int_parser_rules         simple_int_rules;
        detail::complex_variable_rule<double>   name_property_rule;
        detail::complex_variable_rule<double>   id_empire_location_rule;
        detail::complex_variable_rule<double>   name_empire_location_rule;
        detail::complex_variable_rule<double>   empire_meter_value;
        detail::complex_variable_rule<double>   empire_stockpile;
        detail::complex_variable_rule<double>   direct_distance;
        detail::complex_variable_rule<double>   shortest_path;
        detail::complex_variable_rule<double>   species_content_opinion;
        detail::complex_variable_rule<double>   species_empire_opinion;
        detail::complex_variable_rule<double>   species_species_opinion;
        detail::complex_variable_rule<double>   special_capacity;
        detail::complex_variable_rule<double>   unwrapped_part_meter;
        detail::complex_variable_rule<double>   value_wrapped_part_meter;
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
        detail::value_ref_rule<double>      int_total_fighter_shots_cast;
        detail::value_ref_rule<double>      named_real_valueref;
        detail::value_ref_rule<double>      named_int_valueref_cast;
    };

    struct castable_as_int_parser_rules {
        castable_as_int_parser_rules(const lexer& tok,
                                     detail::Labeller& label,
                                     const detail::condition_parser_grammar& condition_parser,
                                     const detail::value_ref_grammar<std::string>& string_grammar);

        int_arithmetic_rules                int_rules;
        double_parser_rules                 double_rules;
        detail::planet_type_parser_rules    planet_type_rules;
        detail::value_ref_rule<int>         castable_expr;
        detail::value_ref_rule<int>         enum_expr;
        detail::value_ref_rule<int>         flexible_int;
        detail::value_ref_rule<int>         enum_or_int;
    };
}


#endif
