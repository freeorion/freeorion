// -*- C++ -*-
#include "ValueRefParser.h"

#include "ConditionParserImpl.h"
#include "EnumParser.h"
#include "Label.h"
#include "../universe/ValueRef.h"

#include <boost/spirit/home/phoenix.hpp>


namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;


#define DEBUG_VALUEREF_PARSERS 0

// These are just here to satisfy the requirements of qi::debug(<rule>).
#if DEBUG_VALUEREF_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const std::vector<adobe::name_t>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::vector<boost::variant<ValueRef::OpType, ValueRef::ValueRefBase<int>*> >&) { return os; }
    inline ostream& operator<<(ostream& os, const std::vector<boost::variant<ValueRef::OpType, ValueRef::ValueRefBase<double>*> >&) { return os; }
}
#endif

typedef qi::rule<
    parse::token_iterator,
    adobe::name_t (),
    parse::skipper_type
> name_token_rule;

template <typename T>
struct variable_rule
{
    typedef qi::rule<
        parse::token_iterator,
        ValueRef::Variable<T>* (),
        qi::locals<std::vector<adobe::name_t> >,
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
            std::vector<adobe::name_t>,
            ValueRef::StatisticType,
            Condition::ConditionBase*
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
                    )
                    >> '(' > expr [ _val = new_<ValueRef::Operation<T> >(_c, _1) ] > ')'
                )
            |   (
                    (
                        tok.Min_    [ _c = ValueRef::MINIMUM ]
                    |   tok.Max_    [ _c = ValueRef::MAXIMUM ]
                    |   tok.Random_ [ _c = ValueRef::RANDOM_UNIFORM ]
                    )
                    >> '(' > expr [ _a = _1 ] > ','
                    > expr [ _val = new_<ValueRef::Operation<T> >(_c, _a, _1) ] > ')'
                )
            |   (
                    primary_expr [ _val = _1 ]
                )
            )
        ;

    exponential_expr
        =   (
                function_expr [ _a = _1 ]
                >> '^' > function_expr
                [ _val = new_<ValueRef::Operation<T> >( ValueRef::EXPONENTIATE, _a, _1 ) ]
            )
        |   function_expr [ _val = _1 ]
        ;

    multiplicative_expr
        =   (
                exponential_expr [ _a = _1 ]
            >   (
                   *(
                        (
                            (
                                lit('*') [ _c = ValueRef::TIMES ]
                            |   lit('/') [ _c = ValueRef::DIVIDE ]
                            )
                        >   exponential_expr [ _b = new_<ValueRef::Operation<T> >(_c, _a, _1) ]
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
                    >   (
                        *(
                                (
                                    (
                                        lit('+') [ _c = ValueRef::PLUS ]
                                    |   lit('-') [ _c = ValueRef::MINUS ]
                                    )
                                >   multiplicative_expr [ _b = new_<ValueRef::Operation<T> >(_c, _a, _1) ]
                                ) [ _a = _b ]
                            )
                        )
                    )
                    [ _val = _a ]
                )
            |   (
                    lit('-') > multiplicative_expr
                    [ _val = new_<ValueRef::Operation<T> >(ValueRef::NEGATE, _1) ]
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

extern name_token_rule              first_token;
extern name_token_rule              container_token;
const name_token_rule&              int_var_final_token();
const statistic_rule<int>::type&    int_var_statistic();
const name_token_rule&              double_var_final_token();
const statistic_rule<double>::type& double_var_statistic();

template <typename T>
void initialize_numeric_statistic_parser(
    typename statistic_rule<T>::type& statistic,
    const name_token_rule& final_token)
{
    const parse::lexer& tok = parse::lexer::instance();

    qi::_1_type _1;
    qi::_a_type _a;
    qi::_b_type _b;
    qi::_c_type _c;
    qi::_val_type _val;
    qi::eps_type eps;
    using phoenix::new_;
    using phoenix::push_back;
    using phoenix::val;

    statistic
        =    (
                  (
                        (
                            tok.Count_ [ _b = ValueRef::COUNT ]
                        |   tok.If_ [ _b = ValueRef::IF ]
                        )
                   >   parse::label(Condition_name) >> parse::detail::condition_parser [ _c = _1 ]
                  )
              |   (
                       parse::enum_parser<ValueRef::StatisticType>() [ _b = _1 ]
                   >>  parse::label(Property_name)
                   >>       eps [ push_back(_a, val(LocalCandidate_name)) ]
                   >>       -(container_token [ push_back(_a, _1) ] >> '.')
                   >>       final_token [ push_back(_a, _1) ]
                   >>  parse::label(Condition_name) >>   parse::detail::condition_parser [ _c = _1 ]
                  )
             )
             [ _val = new_<ValueRef::Statistic<T> >(_a, _b, _c) ]
        ;
}

template <typename T>
void initialize_nonnumeric_statistic_parser(
    typename statistic_rule<T>::type& statistic,
    const name_token_rule& final_token)
{
    const parse::lexer& tok = parse::lexer::instance();

    qi::_1_type _1;
    qi::_a_type _a;
    qi::_b_type _b;
    qi::_c_type _c;
    qi::_val_type _val;
    qi::eps_type eps;
    using phoenix::new_;
    using phoenix::push_back;
    using phoenix::val;

    statistic
        =    (
                  tok.Mode_ [ _b = ValueRef::MODE ]
              >>  parse::label(Property_name)
              >>        eps [ push_back(_a, val(LocalCandidate_name)) ]
              >>        -(container_token [ push_back(_a, _1) ] > '.')
              >>        final_token [ push_back(_a, _1) ]
              >   parse::label(Condition_name) >  parse::detail::condition_parser [ _c = _1 ]
             )
             [ _val = new_<ValueRef::Statistic<T> >(_a, _b, _c) ]
        ;
}
