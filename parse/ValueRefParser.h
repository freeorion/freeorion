#ifndef _ValueRefParser_h_
#define _ValueRefParser_h_

#include "Lexer.h"
#include "ParseImpl.h"

#include "../universe/ValueRefFwd.h"
#include "../universe/EnumsFwd.h"

#include <boost/spirit/include/qi.hpp>

namespace Condition {
    struct ConditionBase;
}

namespace parse {
    template <typename T>
    using value_ref_rule = detail::rule<
        // TODO: Investigate refactoring ValueRef to use variant,
        // for increased locality of reference.
        ValueRef::ValueRefBase<T>* ()
            >;
    typedef detail::rule<
        Condition::ConditionBase* ()
    > condition_parser_rule;
}

namespace parse { namespace detail {

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
    simple_variable_rules(const std::string& type_name);

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
                         const parse::condition_parser_rule& condition_parser);

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

struct simple_int_parser_rules : public parse::detail::simple_variable_rules<int> {
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
    using complex_variable_rule = parse::detail::rule<complex_variable_signature<T>, complex_variable_locals>;
    template <typename T>
    using complex_variable_grammar = parse::detail::grammar<complex_variable_signature<T>, complex_variable_locals>;

}}

namespace parse {
    value_ref_rule<std::string>& string_value_ref();

    struct int_arithmetic_rules : public parse::detail::arithmetic_rules<int> {
        int_arithmetic_rules(
            const parse::lexer& tok,
            const parse::condition_parser_rule& condition_parser);
        detail::simple_int_parser_rules  simple_int_rules;
    };

    struct double_complex_parser_grammar : public detail::complex_variable_grammar<double> {
        double_complex_parser_grammar(const lexer& tok, const condition_parser_rule& condition_parser);

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
            const parse::lexer& tok,
            const parse::condition_parser_rule& condition_parser);

        parse::int_arithmetic_rules        int_rules;
        detail::simple_int_parser_rules    simple_int_rules;
        detail::simple_double_parser_rules simple_double_rules;
        parse::double_complex_parser_grammar double_complex_grammar;
        parse::value_ref_rule<double> int_constant_cast;
        parse::value_ref_rule<double> int_bound_variable_cast;
        parse::value_ref_rule<double> int_free_variable_cast;
        parse::value_ref_rule<double> int_statistic_cast;
        parse::value_ref_rule<double> int_complex_variable_cast;
    };

    struct castable_as_int_parser_rules {
        castable_as_int_parser_rules(const parse::lexer& tok,
                                     const parse::condition_parser_rule& condition_parser);

        parse::int_arithmetic_rules     int_rules;
        parse::double_parser_rules      double_rules;
        parse::value_ref_rule<int> castable_expr;
        parse::value_ref_rule<int> flexible_int;
    };


    namespace detail {

    template <typename T>
    struct enum_value_ref_rules;

    enum_value_ref_rules<PlanetEnvironment>& planet_environment_rules();
    enum_value_ref_rules<PlanetSize>& planet_size_rules();
    enum_value_ref_rules<PlanetType>& planet_type_rules();
    enum_value_ref_rules<StarType>& star_type_rules();
    enum_value_ref_rules<Visibility>& visibility_rules();
    enum_value_ref_rules<UniverseObjectType>& universe_object_type_rules();

    }
}

#endif
