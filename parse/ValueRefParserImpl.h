#include "ValueRefParser.h"

#include "ParseImpl.h"
#include "ConditionParserImpl.h"
#include "EnumParser.h"
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

typedef parse::detail::rule<
    ValueRef::ReferenceType ()
> reference_token_rule;

typedef parse::detail::rule<
    const char* ()
> name_token_rule;

typedef parse::value_ref_rule<int> int_rule;
typedef parse::value_ref_rule<double> double_rule;

template <typename T>
using variable_rule = parse::detail::rule<
    ValueRef::Variable<T>* (),
    qi::locals<
        std::vector<std::string>,
        ValueRef::ReferenceType
    >
>;

template <typename T>
using statistic_rule = parse::detail::rule<
    ValueRef::Statistic<T>* (),
    qi::locals<
        ValueRef::ValueRefBase<T>*,
        ValueRef::StatisticType
    >
>;

template <typename T>
using complex_variable_rule = parse::detail::rule<
    ValueRef::ComplexVariable<T>* (),
    qi::locals<
        std::string,
        ValueRef::ValueRefBase<int>*,
        ValueRef::ValueRefBase<int>*,
        ValueRef::ValueRefBase<std::string>*,
        ValueRef::ValueRefBase<std::string>*,
        ValueRef::ValueRefBase<int>*
    >
>;

template <typename T>
using expression_rule = parse::detail::rule<
    ValueRef::ValueRefBase<T>* (),
    qi::locals<
        ValueRef::ValueRefBase<T>*,
        ValueRef::ValueRefBase<T>*,
        ValueRef::OpType,
        std::vector<ValueRef::ValueRefBase<T>*>
    >
>;

template <typename T>
void initialize_nonnumeric_expression_parsers(
    expression_rule<T>& function_expr,
    expression_rule<T>& operated_expr,
    typename parse::value_ref_rule<T>& expr,
    typename parse::value_ref_rule<T>& primary_expr)
{
    qi::_1_type _1;
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
                    [ _val = new_<ValueRef::Operation<T> >(_c, _d) ] >   ')'
                )
            |   (
                    primary_expr [ _val = _1 ]
                )
            )
        ;

    operated_expr
        =   function_expr   // no operators supported for generic non-numeric types. specialized implementation (eg. std::string) may support some operators, though
        ;

    expr
        =   operated_expr
        ;
}

template <>
void initialize_nonnumeric_expression_parsers<std::string>(
    expression_rule<std::string>& function_expr,
    expression_rule<std::string>& operated_expr,
    typename parse::value_ref_rule<std::string>& expr,
    typename parse::value_ref_rule<std::string>& primary_expr);


template <typename T>
void initialize_numeric_expression_parsers(
    expression_rule<T>& function_expr,
    expression_rule<T>& exponential_expr,
    expression_rule<T>& multiplicative_expr,
    expression_rule<T>& additive_expr,
    typename parse::value_ref_rule<T>& expr,
    typename parse::value_ref_rule<T>& primary_expr)
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
                        tok.Sin_    [ _c = ValueRef::SINE ]                     // single-parameter math functions
                    |   tok.Cos_    [ _c = ValueRef::COSINE ]
                    |   tok.Log_    [ _c = ValueRef::LOGARITHM ]
                    |   tok.Abs_    [ _c = ValueRef::ABS ]
                    )
                    >> '(' >> expr [ _val = new_<ValueRef::Operation<T> >(_c, _1) ] >> ')'
                )
            |   (
                        tok.RandomNumber_   [ _c = ValueRef::RANDOM_UNIFORM ]   // random number requires a min and max value
                    >  '(' >    expr [ _a = _1 ]
                    >  ',' >    expr [ _val = new_<ValueRef::Operation<T> >(_c, _a, _1) ] >> ')'
                )
            |   (
                    (
                        tok.OneOf_  [ _c = ValueRef::RANDOM_PICK ]              // oneof, min, or max can take any number or operands
                    |   tok.Min_    [ _c = ValueRef::MINIMUM ]
                    |   tok.Max_    [ _c = ValueRef::MAXIMUM ]
                    )
                    >>  '(' >>  expr [ push_back(_d, _1) ]
                    >>(*(',' >   expr [ push_back(_d, _1) ] ))
                    [ _val = new_<ValueRef::Operation<T> >(_c, _d) ] >> ')'
                )
            |   (
                    lit('-') >> function_expr                                   // single parameter math function with a function expression rather than any arbitrary expression as parameter, because negating more general expressions can be ambiguous
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
                    >>  (
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

const reference_token_rule&                     variable_scope();
const name_token_rule&                          container_type();
const int_rule&                                 int_constant();
const name_token_rule&                          int_bound_variable_name();
const variable_rule<int>&                       int_bound_variable();
const name_token_rule&                          int_free_variable_name();
const variable_rule<int>&                       int_free_variable();
const statistic_rule<int>&                      int_var_statistic();
const complex_variable_rule<int>&               int_var_complex();
const int_rule&                                 int_simple();
const double_rule&                              double_constant();
const name_token_rule&                          double_bound_variable_name();
const variable_rule<double>&                    double_bound_variable();
const name_token_rule&                          double_free_variable_name();
const variable_rule<double>&                    double_free_variable();
const statistic_rule<double>&                   double_var_statistic();
const complex_variable_rule<double>&            double_var_complex();
const complex_variable_rule<std::string>&       string_var_complex();

template <typename T>
void initialize_bound_variable_parser(
    variable_rule<T>& bound_variable,
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
        =   variable_scope() [ _b = _1 ] >> '.'
        >>-(container_type() [ push_back(_a, construct<std::string>(_1)) ] > '.')
        >>  variable_name    [ push_back(_a, construct<std::string>(_1)), _val = new_<ValueRef::Variable<T> >(_b, _a) ]
        ;
}

template <typename T>
void initialize_numeric_statistic_parser(
    statistic_rule<T>& statistic,
    statistic_rule<T>& statistic_1,
    statistic_rule<T>& statistic_2,
    const typename parse::value_ref_rule<T>& value_ref
    )
{
    const parse::lexer& tok = parse::lexer::instance();

    qi::_1_type _1;
    qi::_a_type _a;
    qi::_b_type _b;
    qi::_val_type _val;
    using phoenix::construct;
    using phoenix::new_;
    using phoenix::push_back;

    statistic_1
        =   tok.Statistic_
            >> (   tok.Count_  [ _b = ValueRef::COUNT ]
               |   tok.If_     [ _b = ValueRef::IF ]
            )
            >   parse::detail::label(Condition_token) >    parse::detail::condition_parser
                [ _val = new_<ValueRef::Statistic<T> >(_a, _b, _1) ]
        ;

    statistic_2
        =   tok.Statistic_
            >>  parse::statistic_type_enum() [ _b = _1 ]
            >   parse::detail::label(Value_token)     >     value_ref [ _a = _1 ]
            >   parse::detail::label(Condition_token) >     parse::detail::condition_parser
                [ _val = new_<ValueRef::Statistic<T> >(_a, _b, _1) ]
        ;

    statistic
        =   statistic_1
        |   statistic_2
        ;
}

template <typename T>
void initialize_nonnumeric_statistic_parser(
    statistic_rule<T>& statistic,
    const typename parse::value_ref_rule<T>& value_ref)
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
        =   tok.Statistic_
            >>  tok.Mode_ [ _b = ValueRef::MODE ]
            >   parse::detail::label(Value_token)     >     value_ref [ _a = _1 ]
            >   parse::detail::label(Condition_token) >     parse::detail::condition_parser
                [ _val = new_<ValueRef::Statistic<T> >(_a, _b, _1) ]
        ;
}
