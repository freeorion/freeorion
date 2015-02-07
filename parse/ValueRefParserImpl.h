// -*- C++ -*-
#include "ValueRefParser.h"

#include "ConditionParserImpl.h"
#include "EnumParser.h"
#include "Label.h"
#include "../universe/ValueRef.h"

#include <boost/spirit/include/phoenix.hpp>


namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;


#define DEBUG_VALUEREF_PARSERS 0

// These are just here to satisfy the requirements of qi::debug(<rule>).
#if DEBUG_VALUEREF_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const std::vector<const char*>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::vector<boost::variant<ValueRef::OpType, ValueRef::ValueRefBase<int>*> >&) { return os; }
    inline ostream& operator<<(ostream& os, const std::vector<boost::variant<ValueRef::OpType, ValueRef::ValueRefBase<double>*> >&) { return os; }
}
#endif

typedef qi::rule<
    parse::token_iterator,
    ValueRef::ReferenceType (),
    parse::skipper_type
> reference_token_rule;

typedef qi::rule<
    parse::token_iterator,
    const char* (),
    parse::skipper_type
> name_token_rule;

typedef parse::value_ref_parser_rule<int>::type     int_rule;
typedef parse::value_ref_parser_rule<double>::type  double_rule;

template <typename T>
struct variable_rule
{
    typedef qi::rule<
        parse::token_iterator,
        ValueRef::Variable<T>* (),
        qi::locals<
            std::vector<std::string>,
            ValueRef::ReferenceType
        >,
        parse::skipper_type
    > type;
};

template <typename T>
struct statistic_rule
{
    typedef qi::rule<
        parse::token_iterator,
        ValueRef::Statistic<T>* (),
        qi::locals<
            std::vector<std::string>,
            ValueRef::StatisticType,
            ValueRef::ValueRefBase<T>*
        >,
        parse::skipper_type
    > type;
};

template <typename T>
struct complex_variable_rule
{
    typedef qi::rule<
        parse::token_iterator,
        ValueRef::ComplexVariable<T>* (),
        qi::locals<
            std::string,
            ValueRef::ValueRefBase<int>*,
            ValueRef::ValueRefBase<int>*,
            ValueRef::ValueRefBase<std::string>*,
            ValueRef::ValueRefBase<std::string>*,
            ValueRef::ValueRefBase<int>*
        >,
        parse::skipper_type
    > type;
};

template <typename T>
struct expression_rule
{
    typedef qi::rule<
        parse::token_iterator,
        ValueRef::ValueRefBase<T>* (),
        qi::locals<
            ValueRef::ValueRefBase<T>*,
            ValueRef::ValueRefBase<T>*,
            ValueRef::OpType
        >,
        parse::skipper_type
    > type;
};

template <typename T>
void initialize_expression_parsers(
    typename expression_rule<T>::type&              function_expr,
    typename expression_rule<T>::type&              exponential_expr,
    typename expression_rule<T>::type&              multiplicative_expr,
    typename expression_rule<T>::type&              additive_expr,
    typename parse::value_ref_parser_rule<T>::type& expr,
    typename parse::value_ref_parser_rule<T>::type& primary_expr         )
{
    qi::_1_type _1;
    qi::_a_type _a;
    qi::_b_type _b;
    qi::_c_type _c;
    qi::_r1_type _r1;
    qi::_val_type _val;
    qi::lit_type lit;
    using phoenix::new_;
    using phoenix::push_back;

    const parse::lexer& tok = parse::lexer::instance();

    function_expr
        =   (
                (
                    (
                        tok.Sin_    [ _c = ValueRef::SINE ]
                    |   tok.Cos_    [ _c = ValueRef::COSINE ]
                    |   tok.Log_    [ _c = ValueRef::LOGARITHM ]
                    |   tok.Abs_    [ _c = ValueRef::ABS ]
                    )
                    >> '(' >> expr [ _val = new_<ValueRef::Operation<T> >(_c, _1) ] >> ')'
                )
            |   (
                    (
                        tok.Min_            [ _c = ValueRef::MINIMUM ]
                    |   tok.Max_            [ _c = ValueRef::MAXIMUM ]
                    |   tok.RandomNumber_   [ _c = ValueRef::RANDOM_UNIFORM ]
                    )
                    >> '(' >> expr [ _a = _1 ] >> ','
                    >> expr [ _val = new_<ValueRef::Operation<T> >(_c, _a, _1) ] >> ')'
                )
            |   (
                    lit('-') >> function_expr
                    [ _val = new_<ValueRef::Operation<T> >(ValueRef::NEGATE, _1) ]
                )
            |   (
                    primary_expr [ _val = _1 ]
                )
            )
        ;

    exponential_expr
        =   (
                function_expr [ _a = _1 ]
                >> '^' >> function_expr
                [ _val = new_<ValueRef::Operation<T> >( ValueRef::EXPONENTIATE, _a, _1 ) ]
            )
        |   function_expr [ _val = _1 ]
        ;

    multiplicative_expr
        =   (
                exponential_expr [ _a = _1 ]
            >>   (
                   *(
                        (
                            (
                                lit('*') [ _c = ValueRef::TIMES ]
                            |   lit('/') [ _c = ValueRef::DIVIDE ]
                            )
                        >>   exponential_expr [ _b = new_<ValueRef::Operation<T> >(_c, _a, _1) ]
                        ) [ _a = _b ]
                    )
                )
            )
            [ _val = _a ]
        ;

    additive_expr
        =
            (
                (
                    (
                        multiplicative_expr [ _a = _1 ]
                    >>   (
                        *(
                                (
                                    (
                                        lit('+') [ _c = ValueRef::PLUS ]
                                    |   lit('-') [ _c = ValueRef::MINUS ]
                                    )
                                >>   multiplicative_expr [ _b = new_<ValueRef::Operation<T> >(_c, _a, _1) ]
                                ) [ _a = _b ]
                            )
                        )
                    )
                    [ _val = _a ]
                )
            |   (
                    multiplicative_expr [ _val = _1 ]
                )
            )
        ;

    expr
        =   additive_expr
        ;
}

const reference_token_rule&                 variable_scope();
const name_token_rule&                      container_type();
const int_rule&                             int_constant();
const name_token_rule&                      int_bound_variable_name();
const variable_rule<int>::type&             int_bound_variable();
const name_token_rule&                      int_free_variable_name();
const variable_rule<int>::type&             int_free_variable();
const statistic_rule<int>::type&            int_var_statistic();
const complex_variable_rule<int>::type&     int_var_complex();
const int_rule&                             int_simple();
const double_rule&                          double_constant();
const name_token_rule&                      double_bound_variable_name();
const variable_rule<double>::type&          double_bound_variable();
const name_token_rule&                      double_free_variable_name();
const variable_rule<double>::type&          double_free_variable();
const statistic_rule<double>::type&         double_var_statistic();
const complex_variable_rule<double>::type&  double_var_complex();

template <typename T>
void initialize_bound_variable_parser(
    typename variable_rule<T>::type& bound_variable,
    const name_token_rule& variable_name)
{
    qi::_1_type _1;
    qi::_a_type _a;
    qi::_b_type _b;
    qi::_val_type _val;
    using phoenix::construct;
    using phoenix::new_;
    using phoenix::push_back;

    bound_variable
        =   variable_scope() [ _b = _1 ] > '.'
        > -(container_type() [ push_back(_a, construct<std::string>(_1)) ] > '.')
        >   variable_name    [ push_back(_a, construct<std::string>(_1)), _val = new_<ValueRef::Variable<T> >(_b, _a) ]
        ;
}

template <typename T>
void initialize_numeric_statistic_parser(
    typename statistic_rule<T>::type& statistic,
    typename statistic_rule<T>::type& statistic_1,
    typename statistic_rule<T>::type& statistic_2,
    typename statistic_rule<T>::type& statistic_3,
    const name_token_rule& variable_name,
    const typename parse::value_ref_parser_rule<T>::type& constant,
    const typename variable_rule<T>::type& free_variable,
    const typename variable_rule<T>::type& bound_variable,
    const typename variable_rule<T>::type& complex_variable
    )
{
    const parse::lexer& tok = parse::lexer::instance();

    qi::_1_type _1;
    qi::_a_type _a;
    qi::_b_type _b;
    qi::_c_type _c;
    qi::_val_type _val;
    using phoenix::construct;
    using phoenix::new_;
    using phoenix::push_back;

    statistic_1
        =   (   tok.Count_  [ _b = ValueRef::COUNT ]
            |   tok.If_     [ _b = ValueRef::IF ] )
            >   parse::label(Condition_token) >    parse::detail::condition_parser
                [ _val = new_<ValueRef::Statistic<T> >(_a, _b, _1) ]
        ;

    statistic_2
        =       parse::enum_parser<ValueRef::StatisticType>() [ _b = _1 ]
            >>  parse::label(Property_token)
            >>  -(container_type() [ push_back(_a, construct<std::string>(_1)) ] >> '.')
            >>  variable_name [ push_back(_a, construct<std::string>(_1)) ]
            >   parse::label(Condition_token) >     parse::detail::condition_parser
                [ _val = new_<ValueRef::Statistic<T> >(_a, _b, _1) ]
        ;

    statistic_3
        =       parse::enum_parser<ValueRef::StatisticType>() [ _b = _1 ]
            >>  parse::label(Value_token)
            >> (    constant        [_c = _1 ]  // match only a subset of ValueRef types here, because parsing as a top-level
               |    free_variable   [_c = _1 ]  // ValueRef here causes deadlocks on parser init on some systems
               |    bound_variable  [_c = _1 ]  // todo: try allowing parsing of complex variables here
               |    complex_variable[_c = _1 ]
               )
            >   parse::label(Condition_token) >    parse::detail::condition_parser
                [ _val = new_<ValueRef::Statistic<T> >(_c, _b, _1) ]
        ;

    statistic
        %=  statistic_1
        |   statistic_2
        |   statistic_3
        ;
}

template <typename T>
void initialize_nonnumeric_statistic_parser(
    typename statistic_rule<T>::type& statistic,
    const name_token_rule& variable_name)
{
    const parse::lexer& tok = parse::lexer::instance();

    qi::_1_type _1;
    qi::_a_type _a;
    qi::_b_type _b;
    qi::_val_type _val;
    using phoenix::construct;
    using phoenix::new_;
    using phoenix::push_back;

    statistic
        =       tok.Mode_ [ _b = ValueRef::MODE ]
            >>  parse::label(Property_token)
            >> -(container_type() [ push_back(_a, construct<std::string>(_1)) ] > '.')
            >>  variable_name [ push_back(_a, construct<std::string>(_1)) ]
            >>  parse::label(Condition_token) >> parse::detail::condition_parser
                [ _val = new_<ValueRef::Statistic<T> >(_a, _b, _1) ]
        ;
}
