#include "ValueRefParser.h"

#include "ConditionParserImpl.h"
#include "MovableEnvelope.h"
#include "../universe/ValueRef.h"

#include <boost/spirit/include/phoenix.hpp>


#define DEBUG_VALUEREF_PARSERS 0

// These are just here to satisfy the requirements of boost::spirit::qi::debug(<rule>).
#if DEBUG_VALUEREF_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const std::vector<const char*>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::vector<boost::variant<ValueRef::OpType, value_ref_payload<int>>>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::vector<boost::variant<ValueRef::OpType, value_ref_payload<double>>>&) { return os; }
}
#endif

const parse::detail::reference_token_rule                      variable_scope(const parse::lexer& tok);
const parse::detail::name_token_rule                           container_type(const parse::lexer& tok);

template <typename T>
void initialize_nonnumeric_statistic_parser(
    parse::detail::statistic_rule<T>& statistic,
    const parse::lexer& tok,
    parse::detail::Labeller& labeller,
    const parse::detail::condition_parser_grammar& condition_parser,
    const typename parse::detail::value_ref_rule<T>& value_ref)
{
    using boost::phoenix::construct;
    using boost::phoenix::new_;
    using boost::phoenix::push_back;

    boost::spirit::qi::_1_type _1;
    boost::spirit::qi::_a_type _a;
    boost::spirit::qi::_b_type _b;
    boost::spirit::qi::_val_type _val;
    boost::spirit::qi::_pass_type _pass;
    const boost::phoenix::function<parse::detail::construct_movable> construct_movable_;
    const boost::phoenix::function<parse::detail::deconstruct_movable> deconstruct_movable_;

    statistic
        =   (tok.Statistic_ >>  tok.Mode_ [ _b = ValueRef::MODE ])
            >   labeller.rule(Value_token)     >     value_ref [ _a = _1 ]
            >   labeller.rule(Condition_token) >     condition_parser
        [ _val = construct_movable_(new_<ValueRef::Statistic<T>>(deconstruct_movable_(_a, _pass), _b, deconstruct_movable_(_1, _pass))) ]
        ;
}


template <typename T>
void initialize_bound_variable_parser(
    parse::detail::variable_rule<T>& bound_variable,
    const parse::detail::name_token_rule& variable_name,
    const parse::detail::reference_token_rule& variable_scope_rule,
    const parse::detail::name_token_rule& container_type_rule)
{
    using boost::phoenix::construct;
    using boost::phoenix::new_;
    using boost::phoenix::push_back;

    boost::spirit::qi::_1_type _1;
    boost::spirit::qi::_a_type _a;
    boost::spirit::qi::_b_type _b;
    boost::spirit::qi::_val_type _val;
    const boost::phoenix::function<parse::detail::construct_movable> construct_movable_;

    bound_variable
        =   variable_scope_rule [ _b = _1 ] >>
        '.' >>-(container_type_rule [ push_back(_a, construct<std::string>(_1)) ] > '.')
            >>  variable_name
        [ push_back(_a, construct<std::string>(_1)), _val = construct_movable_(new_<ValueRef::Variable<T>>(_b, _a)) ]
        ;
}


namespace parse { namespace detail {

template <typename T>
enum_value_ref_rules<T>::enum_value_ref_rules(const std::string& type_name,
                                              const parse::lexer& tok,
                                              Labeller& labeller,
                                              const condition_parser_grammar& condition_parser)
{
        using boost::phoenix::new_;
        using boost::phoenix::push_back;

        boost::spirit::qi::_1_type _1;
        boost::spirit::qi::_c_type _c;
        boost::spirit::qi::_d_type _d;
        boost::spirit::qi::_val_type _val;
        boost::spirit::qi::_pass_type _pass;
        boost::spirit::qi::lit_type lit;
        const boost::phoenix::function<parse::detail::construct_movable> construct_movable_;
        const boost::phoenix::function<parse::detail::deconstruct_movable> deconstruct_movable_;
        const boost::phoenix::function<parse::detail::deconstruct_movable_vector> deconstruct_movable_vector_;

        constant_expr
            =   enum_expr [ _val = construct_movable_(new_<ValueRef::Constant<T>>(_1)) ]
            ;

        variable_scope_rule = variable_scope(tok);
        container_type_rule = container_type(tok);
        initialize_bound_variable_parser<T>(bound_variable_expr, variable_name, variable_scope_rule, container_type_rule);

        statistic_sub_value_ref
            =   constant_expr
            |   bound_variable_expr
            |   free_variable_expr
            |   complex_expr
            ;

        functional_expr
            =   (
                    (
                        (
                            tok.OneOf_  [ _c = ValueRef::RANDOM_PICK ]
                            |   tok.Min_    [ _c = ValueRef::MINIMUM ]
                            |   tok.Max_    [ _c = ValueRef::MAXIMUM ]
                        )
                        >
                        ( '('
                          >   expr [ push_back(_d, _1) ]
                          > *(','  >   expr [ push_back(_d, _1) ] )
                        > ')'
                        )
                        [ _val = construct_movable_(new_<ValueRef::Operation<T>>(_c, deconstruct_movable_vector_(_d, _pass))) ]
                    )
                |   (
                        primary_expr [ _val = _1 ]
                    )
                )
            ;

        expr
            =   functional_expr
            ;

        initialize_nonnumeric_statistic_parser<T>(
            statistic_expr, tok, labeller, condition_parser, statistic_sub_value_ref);

        primary_expr
            =   constant_expr
            |   free_variable_expr
            |   bound_variable_expr
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


template <typename T>
simple_variable_rules<T>::simple_variable_rules(const std::string& type_name,
                                                const parse::lexer& tok)
{
        using boost::phoenix::new_;

        boost::spirit::qi::_1_type _1;
        boost::spirit::qi::_val_type _val;
        const boost::phoenix::function<parse::detail::construct_movable> construct_movable_;

        free_variable
            =   tok.Value_
            [ _val = construct_movable_(new_<ValueRef::Variable<T>>(ValueRef::EFFECT_TARGET_VALUE_REFERENCE)) ]
            |   free_variable_name
            [ _val = construct_movable_(new_<ValueRef::Variable<T>>(ValueRef::NON_OBJECT_REFERENCE, _1)) ]
            ;

        simple
            =   constant
            |   free_variable
            |   bound_variable
            ;

        variable_scope_rule = variable_scope(tok);
        container_type_rule = container_type(tok);
        initialize_bound_variable_parser<T>(bound_variable, bound_variable_name, variable_scope_rule, container_type_rule);

#if DEBUG_VALUEREF_PARSERS
        debug(bound_variable_name);
        debug(free_variable_name);
        debug(constant);
        debug(free_variable);
        debug(bound_variable);
        debug(simple);
#endif

        bound_variable_name.name(type_name + " bound variable name");
        free_variable_name.name(type_name + " free variable name");
        constant.name(type_name + " constant");
        free_variable.name(type_name + " free variable");
        bound_variable.name(type_name + " bound variable");
        simple.name(type_name + " simple variable expression");
    }
} }

template <typename T>
parse::detail::arithmetic_rules<T>::arithmetic_rules(
    const std::string& type_name,
    const parse::lexer& tok,
    parse::detail::Labeller& labeller,
    const parse::detail::condition_parser_grammar& condition_parser
) :
    statistic_type_enum(tok)
{
    using boost::phoenix::construct;
    using boost::phoenix::new_;
    using boost::phoenix::push_back;

    boost::spirit::qi::_1_type _1;
    boost::spirit::qi::_2_type _2;
    boost::spirit::qi::_a_type _a;
    boost::spirit::qi::_b_type _b;
    boost::spirit::qi::_c_type _c;
    boost::spirit::qi::_d_type _d;
    boost::spirit::qi::_val_type _val;
    boost::spirit::qi::lit_type lit;
    boost::spirit::qi::_pass_type _pass;
    const boost::phoenix::function<construct_movable> construct_movable_;
    const boost::phoenix::function<deconstruct_movable> deconstruct_movable_;
    const boost::phoenix::function<deconstruct_movable_vector> deconstruct_movable_vector_;

    functional_expr
        =   (
            (
                (
                    tok.Sin_    [ _c = ValueRef::SINE ]                     // single-parameter math functions
                    |   tok.Cos_    [ _c = ValueRef::COSINE ]
                    |   tok.Log_    [ _c = ValueRef::LOGARITHM ]
                    |   tok.Abs_    [ _c = ValueRef::ABS ]
                )
                >> ('(' > expr > ')') [ _val = construct_movable_(new_<ValueRef::Operation<T>>(_c, deconstruct_movable_(_1, _pass))) ]
            )
            |   (
                tok.RandomNumber_   [ _c = ValueRef::RANDOM_UNIFORM ]   // random number requires a min and max value
                >  ( '(' > expr >  ',' > expr > ')' ) [ _val = construct_movable_(
                        new_<ValueRef::Operation<T>>(_c, deconstruct_movable_(_1, _pass), deconstruct_movable_(_2, _pass))) ]
            )
            |   (
                (
                    tok.OneOf_  [ _c = ValueRef::RANDOM_PICK ]              // oneof, min, or max can take any number or operands
                    |   tok.Min_    [ _c = ValueRef::MINIMUM ]
                    |   tok.Max_    [ _c = ValueRef::MAXIMUM ]
                )
                >>  ( '(' >>  expr [ push_back(_d, _1) ]
                >>(*(',' >  expr [ push_back(_d, _1) ] )) >> ')' )
                [ _val = construct_movable_(new_<ValueRef::Operation<T>>(_c, deconstruct_movable_vector_(_d, _pass))) ]
            )
            |   (
                lit('(') >> expr [ push_back(_d, _1) ]
                >> (
                    (     lit("==")   [ _c = ValueRef::COMPARE_EQUAL ]
                          | lit('=')    [ _c = ValueRef::COMPARE_EQUAL ]
                          | lit(">=")   [ _c = ValueRef::COMPARE_GREATER_THAN_OR_EQUAL ]
                          | lit('>')    [ _c = ValueRef::COMPARE_GREATER_THAN ]
                          | lit("<=")   [ _c = ValueRef::COMPARE_LESS_THAN_OR_EQUAL ]
                          | lit('<')    [ _c = ValueRef::COMPARE_LESS_THAN ]
                          | lit("!=")   [ _c = ValueRef::COMPARE_NOT_EQUAL ]
                    )
                    > expr [ push_back(_d, _1) ]
                )
                > (
                    lit(')')
                    | (
                        (lit('?') > expr [ push_back(_d, _1) ])
                        > (
                            lit(')')
                            |  ( lit(':') > expr [ push_back(_d, _1) ] > ')' )
                        )
                    )
                ) [ _val = construct_movable_(new_<ValueRef::Operation<T>>(_c, deconstruct_movable_vector_(_d, _pass))) ]
            )
            |   (
                lit('-') >> functional_expr
                // single parameter math function with a function expression
                // rather than any arbitrary expression as parameter, because
                // negating more general expressions can be ambiguous
                [ _val = construct_movable_(new_<ValueRef::Operation<T>>(ValueRef::NEGATE, deconstruct_movable_(_1, _pass))) ]
            )
            |   (
                primary_expr [ _val = _1 ]
            )
        )
        ;

    exponential_expr
        =   (
            functional_expr [ _a = _1 ]
            >>
            -( '^'
                  >> functional_expr [
                      _b = construct_movable_(new_<ValueRef::Operation<T>>(
                          ValueRef::EXPONENTIATE,
                          deconstruct_movable_(_a, _pass),
                          deconstruct_movable_(_1, _pass) )) ,
                      _a = _b]
            )
        ) [ _val = _a ]
        ;

    multiplicative_expr
        =   (
            exponential_expr [ _a = _1 ]
            >>
            *(
                (
                    (
                        lit('*') [ _c = ValueRef::TIMES ]
                        |   lit('/') [ _c = ValueRef::DIVIDE ]
                    )
                    >>  exponential_expr [
                        _b = construct_movable_(new_<ValueRef::Operation<T>>(
                            _c,
                            deconstruct_movable_(_a, _pass),
                            deconstruct_movable_(_1, _pass))) ]
                ) [ _a = _b ]
            )
            ) [ _val = _a ]
        ;

    additive_expr
        =   (
            multiplicative_expr [ _a = _1 ]
            >>
            *(
                (
                    (
                        lit('+') [ _c = ValueRef::PLUS ]
                        |   lit('-') [ _c = ValueRef::MINUS ]
                    )
                    >>   multiplicative_expr [
                        _b = construct_movable_(new_<ValueRef::Operation<T>>(
                            _c,
                            deconstruct_movable_(_a, _pass),
                            deconstruct_movable_(_1, _pass))) ]
                ) [ _a = _b ]
            )
        ) [ _val = _a ]
      ;

    statistic_collection_expr
        =   (tok.Statistic_
             >> (   tok.Count_  [ _b = ValueRef::COUNT ]
                    |   tok.If_     [ _b = ValueRef::IF ]
                )
            )
        >   labeller.rule(Condition_token) >    condition_parser
        [ _val = construct_movable_(new_<ValueRef::Statistic<T>>(deconstruct_movable_(_a, _pass), _b, deconstruct_movable_(_1, _pass))) ]
        ;

    statistic_value_expr
        =   (tok.Statistic_ >>  statistic_type_enum [ _b = _1 ])
        >   labeller.rule(Value_token)     >     statistic_value_ref_expr [ _a = _1 ]
        >   labeller.rule(Condition_token) >     condition_parser
        [ _val = construct_movable_(new_<ValueRef::Statistic<T>>(deconstruct_movable_(_a, _pass), _b, deconstruct_movable_(_1, _pass))) ]
        ;

    statistic_expr
        =   statistic_collection_expr
        |   statistic_value_expr
        ;

    expr
        =   additive_expr
        ;

#if DEBUG_VALUEREF_PARSERS
    debug(functional_expr);
    debug(exponential_expr);
    debug(multiplicative_expr);
    debug(additive_expr);
    debug(primary_expr);
    debug(statistic_value_ref_expr);
    debug(statistic_collection_expr);
    debug(statistic_value_expr);
    debug(statistic_expr);
    debug(expr);
#endif

    functional_expr.name(type_name + " function expression");
    exponential_expr.name(type_name + " exponential expression");
    multiplicative_expr.name(type_name + " multiplication expression");
    additive_expr.name(type_name + " additive expression");
    statistic_value_ref_expr.name(type_name + " statistic value reference");
    statistic_collection_expr.name(type_name + " collection statistic");
    statistic_value_expr.name(type_name + " value statistic");
    statistic_expr.name("real number statistic");
    primary_expr.name(type_name + " expression");
    expr.name(type_name + " expression");
}
