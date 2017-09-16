#ifndef _ValueRefParser_h_
#define _ValueRefParser_h_

#include "Lexer.h"
#include "ParseImpl.h"

#include "../universe/ValueRefFwd.h"
#include "../universe/EnumsFwd.h"

#include <boost/spirit/include/qi.hpp>


namespace parse {
    template <typename T>
    using value_ref_rule = detail::rule<
        // TODO: Investigate refactoring ValueRef to use variant,
        // for increased locality of reference.
        ValueRef::ValueRefBase<T>* ()
            >;
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
        arithmetic_rules(const std::string& type_name);

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

struct simple_double_parser_rules :
    public simple_variable_rules<double>
{
    simple_double_parser_rules();
};

}}

namespace parse {
    value_ref_rule<int>& int_value_ref();
    value_ref_rule<int>& flexible_int_value_ref();

    value_ref_rule<std::string>& string_value_ref();

    struct double_parser_rules : public detail::arithmetic_rules<double> {
        double_parser_rules();

        detail::simple_double_parser_rules simple_double_rules;
        parse::value_ref_rule<double> int_constant_cast;
        parse::value_ref_rule<double> int_bound_variable_cast;
        parse::value_ref_rule<double> int_free_variable_cast;
        parse::value_ref_rule<double> int_statistic_cast;
        parse::value_ref_rule<double> int_complex_variable_cast;
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
