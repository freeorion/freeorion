#ifndef _ValueRefParser_h_
#define _ValueRefParser_h_

#include "EnumParser.h"

#include "../universe/ValueRefFwd.h"
#include "../universe/EnumsFwd.h"

#include <boost/spirit/include/qi.hpp>

namespace Condition {
    struct ConditionBase;
}

namespace parse {
    // TODO: Investigate refactoring ValueRef to use variant,
    // for increased locality of reference.
    template <typename T>
    using value_ref_signature = ValueRef::ValueRefBase<T>* ();
    template <typename T>
    using value_ref_rule = detail::rule<value_ref_signature<T>>;
    template <typename T>
    using value_ref_grammar = detail::grammar<value_ref_signature<T>>;
}

namespace parse { namespace detail {

    using condition_signature      = Condition::ConditionBase* ();
    using condition_parser_rule    = rule<condition_signature>;
    using condition_parser_grammar = grammar<condition_signature>;

    using name_token_rule = rule<const char* ()>;
    using reference_token_rule = rule<ValueRef::ReferenceType ()>;

    template <typename T>
    using variable_rule = rule<
        ValueRef::Variable<T>* (),
        boost::spirit::qi::locals<
            std::vector<std::string>,
            ValueRef::ReferenceType
            >
        >;

    template <typename T>
    struct simple_variable_rules
    {
        simple_variable_rules(const std::string& type_name, const parse::lexer& tok);

        name_token_rule bound_variable_name;
        name_token_rule free_variable_name;
        parse::value_ref_rule<T> constant;
        variable_rule<T> free_variable;
        variable_rule<T> bound_variable;
        parse::value_ref_rule<T> simple;
        reference_token_rule variable_scope_rule;
        name_token_rule container_type_rule;
    };

    template <typename T>
    using statistic_rule = rule<
        ValueRef::Statistic<T>* (),
        boost::spirit::qi::locals<
            ValueRef::ValueRefBase<T>*,
            ValueRef::StatisticType
            >
        >;

    template <typename T>
    using expression_rule = rule<
        ValueRef::ValueRefBase<T>* (),
        boost::spirit::qi::locals<
            ValueRef::ValueRefBase<T>*,
            ValueRef::ValueRefBase<T>*,
            ValueRef::OpType,
            std::vector<ValueRef::ValueRefBase<T>*>
            >
        >;

    template <typename T>
    struct arithmetic_rules {
        arithmetic_rules(const std::string& type_name,
                         const parse::lexer& tok,
                         Labeller& labeller,
                         const condition_parser_grammar& condition_parser);

        parse::statistic_enum_grammar statistic_type_enum;
        expression_rule<T> functional_expr;
        expression_rule<T> exponential_expr;
        expression_rule<T> multiplicative_expr;
        expression_rule<T> additive_expr;
        parse::value_ref_rule<T> primary_expr;
        parse::value_ref_rule<T> statistic_value_ref_expr;
        statistic_rule<T> statistic_collection_expr;
        statistic_rule<T> statistic_value_expr;
        statistic_rule<T> statistic_expr;
        parse::value_ref_rule<T> expr;
    };

    struct simple_int_parser_rules : public simple_variable_rules<int> {
        simple_int_parser_rules(const parse::lexer& tok);
    };

    struct simple_double_parser_rules : public simple_variable_rules<double> {
        simple_double_parser_rules(const parse::lexer& tok);
    };

    template <typename T>
    using complex_variable_signature = ValueRef::ComplexVariable<T>* ();
    using complex_variable_locals =
        boost::spirit::qi::locals<
        std::string,
        ValueRef::ValueRefBase<int>*,
        ValueRef::ValueRefBase<int>*,
        ValueRef::ValueRefBase<std::string>*,
        ValueRef::ValueRefBase<std::string>*,
        ValueRef::ValueRefBase<int>*
        >;

    template <typename T>
    using complex_variable_rule = rule<complex_variable_signature<T>, complex_variable_locals>;
    template <typename T>
    using complex_variable_grammar = grammar<complex_variable_signature<T>, complex_variable_locals>;

    struct string_complex_parser_grammar : public complex_variable_grammar<std::string> {
        string_complex_parser_grammar(const parse::lexer& tok,
                                      Labeller& labeller,
                                      const parse::value_ref_grammar<std::string>& string_grammar);

        simple_int_parser_rules  simple_int_rules;
        complex_variable_rule<std::string> game_rule;

        complex_variable_rule<std::string> empire_ref;
        complex_variable_rule<std::string> empire_empire_ref;

        complex_variable_rule<std::string> start;
    };

}}

namespace parse {
    struct string_parser_grammar : public value_ref_grammar<std::string> {
        string_parser_grammar(const lexer& tok,
                              detail::Labeller& labeller,
                              const detail::condition_parser_grammar& condition_parser);

        detail::string_complex_parser_grammar string_var_complex;
        detail::name_token_rule bound_variable_name;
        value_ref_rule<std::string> constant;
        value_ref_rule<std::string> free_variable;
        detail::variable_rule<std::string> bound_variable;
        value_ref_rule<std::string> statistic_sub_value_ref;
        detail::statistic_rule<std::string> statistic;
        detail::expression_rule<std::string> function_expr;
        detail::expression_rule<std::string> operated_expr;
        value_ref_rule<std::string> expr;
        value_ref_rule<std::string> primary_expr;
        detail::reference_token_rule variable_scope_rule;
        detail::name_token_rule container_type_rule;
    };

    struct int_arithmetic_rules;

    struct int_complex_parser_grammar : public detail::complex_variable_grammar<int> {
        int_complex_parser_grammar(const lexer& tok,
                                   detail::Labeller& labeller,
                                   const int_arithmetic_rules& _int_arith_rules,
                                   const value_ref_grammar<std::string>& string_grammar);

        const int_arithmetic_rules& int_rules;
        ship_part_class_enum_grammar ship_part_class_enum;
        detail::complex_variable_rule<int> game_rule;
        detail::complex_variable_rule<int> empire_name_ref;
        detail::complex_variable_rule<int> empire_ships_destroyed;
        detail::complex_variable_rule<int> jumps_between;
        //complex_variable_rule<int> jumps_between_by_empire_supply;
        detail::complex_variable_rule<int> outposts_owned;
        detail::complex_variable_rule<int> parts_in_ship_design;
        detail::complex_variable_rule<int> part_class_in_ship_design;
        detail::complex_variable_rule<int> ship_parts_owned;
        detail::complex_variable_rule<int> empire_design_ref;
        detail::complex_variable_rule<int> slots_in_hull;
        detail::complex_variable_rule<int> slots_in_ship_design;
        detail::complex_variable_rule<int> special_added_on_turn;
        detail::complex_variable_rule<int> start;
    };

    struct int_arithmetic_rules : public detail::arithmetic_rules<int> {
        int_arithmetic_rules(
            const lexer& tok,
            detail::Labeller& labeller,
            const detail::condition_parser_grammar& condition_parser,
            const value_ref_grammar<std::string>& string_grammar);
        detail::simple_int_parser_rules  simple_int_rules;
        int_complex_parser_grammar int_complex_grammar;
    };

    struct double_complex_parser_grammar : public detail::complex_variable_grammar<double> {
        double_complex_parser_grammar(const lexer& tok,
                                      detail::Labeller& labeller,
                                      const detail::condition_parser_grammar& condition_parser,
                                      const value_ref_grammar<std::string>& string_grammar);

        detail::simple_int_parser_rules simple_int_rules;
        detail::complex_variable_rule<double> name_property_rule;
        detail::complex_variable_rule<double> empire_meter_value;
        detail::complex_variable_rule<double> direct_distance;
        detail::complex_variable_rule<double> shortest_path;
        detail::complex_variable_rule<double> species_empire_opinion;
        detail::complex_variable_rule<double> species_species_opinion;
        detail::complex_variable_rule<double> special_capacity;
        detail::complex_variable_rule<double> start;
    };

    struct double_parser_rules : public detail::arithmetic_rules<double> {
        double_parser_rules(
            const lexer& tok,
            detail::Labeller& labeller,
            const detail::condition_parser_grammar& condition_parser,
            const value_ref_grammar<std::string>& string_grammar);

        int_arithmetic_rules        int_rules;
        detail::simple_int_parser_rules    simple_int_rules;
        detail::simple_double_parser_rules simple_double_rules;
        int_complex_parser_grammar int_complex_grammar;
        double_complex_parser_grammar double_complex_grammar;
        value_ref_rule<double> int_constant_cast;
        value_ref_rule<double> int_bound_variable_cast;
        value_ref_rule<double> int_free_variable_cast;
        value_ref_rule<double> int_statistic_cast;
        value_ref_rule<double> int_complex_variable_cast;
    };

    struct castable_as_int_parser_rules {
        castable_as_int_parser_rules(const lexer& tok,
                                     detail::Labeller& labeller,
                                     const detail::condition_parser_grammar& condition_parser,
                                     const value_ref_grammar<std::string>& string_grammar);

        int_arithmetic_rules     int_rules;
        double_parser_rules      double_rules;
        value_ref_rule<int> castable_expr;
        value_ref_rule<int> flexible_int;
    };

    namespace detail {

    template <typename T>
    struct enum_value_ref_rules {
        enum_value_ref_rules(const std::string& type_name,
                             const lexer& tok,
                             Labeller& labeller,
                             const condition_parser_grammar& condition_parser);

        name_token_rule variable_name;
        detail::enum_rule<T> enum_expr;
        value_ref_rule<T> constant_expr;
        value_ref_rule<T> free_variable_expr;
        variable_rule<T> bound_variable_expr;
        expression_rule<T> functional_expr;
        value_ref_rule<T> primary_expr;
        value_ref_rule<T> statistic_sub_value_ref;
        statistic_rule<T> statistic_expr;
        complex_variable_rule<T> complex_expr;
        value_ref_rule<T> expr;
        reference_token_rule variable_scope_rule;
        name_token_rule container_type_rule;
    };

    struct planet_environment_parser_rules :
        public detail::enum_value_ref_rules<PlanetEnvironment>
    {
        planet_environment_parser_rules(const lexer& tok,
                                        Labeller& labeller,
                                        const condition_parser_grammar& condition_parser);
    };

    struct planet_size_parser_rules :
        public detail::enum_value_ref_rules<PlanetSize>
    {
        planet_size_parser_rules(const lexer& tok,
                                 Labeller& labeller,
                                 const condition_parser_grammar& condition_parser);
    };

    struct planet_type_parser_rules :
        public detail::enum_value_ref_rules<PlanetType>
    {
        planet_type_parser_rules(const lexer& tok,
                                 Labeller& labeller,
                                 const condition_parser_grammar& condition_parser);
    };

    struct star_type_parser_rules :
        public enum_value_ref_rules<StarType>
    {
        star_type_parser_rules(const lexer& tok,
                               Labeller& labeller,
                               const condition_parser_grammar& condition_parser);
    };

    struct visibility_complex_parser_grammar : public complex_variable_grammar<Visibility> {
        visibility_complex_parser_grammar(const lexer& tok, Labeller& labeller);

        simple_int_parser_rules  simple_int_rules;
        complex_variable_rule<Visibility> empire_object_visibility;
        complex_variable_rule<Visibility> start;
    };

    struct visibility_parser_rules :
        public detail::enum_value_ref_rules<Visibility>
    {
        visibility_parser_rules(const lexer& tok,
                                Labeller& labeller,
                                const condition_parser_grammar& condition_parser);

        visibility_complex_parser_grammar visibility_var_complex_grammar;
    };

    struct universe_object_type_parser_rules :
        public enum_value_ref_rules<UniverseObjectType>
    {
        universe_object_type_parser_rules(const lexer& tok,
                                          Labeller& labeller,
                                          const condition_parser_grammar& condition_parser);
    };

    }
}

#endif
