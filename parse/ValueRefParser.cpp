#include "ValueRefParser.h"

#include "ConditionParserImpl.h"
#include "MovableEnvelope.h"
#include "../universe/ValueRef.h"

#include <boost/spirit/include/phoenix.hpp>


#define DEBUG_VALUEREF_PARSERS 0

// These are just here to satisfy the requirements of boost::spirit::qi::debug(<rule>).
#if DEBUG_VALUEREF_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const std::vector<boost::variant<ValueRef::OpType, value_ref_payload<int>>>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::vector<boost::variant<ValueRef::OpType, value_ref_payload<double>>>&) { return os; }
}
#endif

namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;

namespace parse { namespace detail {

    const reference_token_rule variable_scope(const parse::lexer& tok) {
        qi::_val_type _val;

        reference_token_rule variable_scope;
        variable_scope
            =   tok.Source_         [ _val = ValueRef::SOURCE_REFERENCE ]
            |   tok.Target_         [ _val = ValueRef::EFFECT_TARGET_REFERENCE ]
            |   tok.LocalCandidate_ [ _val = ValueRef::CONDITION_LOCAL_CANDIDATE_REFERENCE ]
            |   tok.RootCandidate_  [ _val = ValueRef::CONDITION_ROOT_CANDIDATE_REFERENCE ]
            ;

        variable_scope.name("Source, Target, LocalCandidate, or RootCandidate");

        return variable_scope;
    }

    const name_token_rule container_type(const parse::lexer& tok) {
        name_token_rule container_type;
        container_type
            =   tok.Planet_
            |   tok.System_
            |   tok.Fleet_
            ;

        container_type.name("Planet, System, or Fleet");

        return container_type;
    }

    template <typename T>
    simple_variable_rules<T>::simple_variable_rules(
        const std::string& type_name, const parse::lexer& tok)
    {
        using phoenix::new_;

        qi::_1_type _1;
        qi::_val_type _val;
        qi::lit_type lit;
        const phoenix::function<construct_movable> construct_movable_;

        free_variable
            =  (tok.Value_ >> !lit('('))
                [ _val = construct_movable_(new_<ValueRef::Variable<T>>(
                    ValueRef::EFFECT_TARGET_VALUE_REFERENCE)) ]
            |   free_variable_name
                [ _val = construct_movable_(new_<ValueRef::Variable<T>>(
                    ValueRef::NON_OBJECT_REFERENCE, _1)) ]
            ;

        simple
            =   constant
            |   bound_variable
            |   free_variable
            ;

        variable_scope_rule = variable_scope(tok);
        container_type_rule = container_type(tok);
        initialize_bound_variable_parser<T>(
            bound_variable, unwrapped_bound_variable,
            value_wrapped_bound_variable, bound_variable_name,
            variable_scope_rule, container_type_rule, tok);

#if DEBUG_VALUEREF_PARSERS
        debug(bound_variable_name);
        debug(free_variable_name);
        debug(constant);
        debug(free_variable);
        debug(bound_variable);
        debug(simple);
#endif

        unwrapped_bound_variable.name(type_name + " unwrapped bound variable name");
        value_wrapped_bound_variable.name(type_name + " value-wrapped bound variable name");
        bound_variable_name.name(type_name + " bound variable name");
        free_variable_name.name(type_name + " free variable name");
        constant.name(type_name + " constant");
        free_variable.name(type_name + " free variable");
        bound_variable.name(type_name + " bound variable");
        simple.name(type_name + " simple variable expression");
    }

    // Explicit instantiation to prevent costly recompilation in multiple units
    template simple_variable_rules<int>::simple_variable_rules(
        const std::string& type_name, const parse::lexer& tok);
    template simple_variable_rules<double>::simple_variable_rules(
        const std::string& type_name, const parse::lexer& tok);

    template <typename T>
    arithmetic_rules<T>::arithmetic_rules(
        const std::string& type_name,
        const parse::lexer& tok,
        parse::detail::Labeller& label,
        const parse::detail::condition_parser_grammar& condition_parser
    ) :
        statistic_type_enum(tok)
    {
        using phoenix::construct;
        using phoenix::new_;
        using phoenix::push_back;

        qi::_1_type _1;
        qi::_2_type _2;
        qi::_a_type _a;
        qi::_b_type _b;
        qi::_c_type _c;
        qi::_d_type _d;
        qi::_val_type _val;
        qi::lit_type lit;
        qi::_pass_type _pass;
        const phoenix::function<construct_movable> construct_movable_;
        const phoenix::function<deconstruct_movable> deconstruct_movable_;
        const phoenix::function<deconstruct_movable_vector> deconstruct_movable_vector_;

        functional_expr
            = (
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
                                tok.OneOf_  [ _c = ValueRef::RANDOM_PICK ] // oneof, min, or max can take any number or operands
                            |   tok.Min_    [ _c = ValueRef::MINIMUM ]
                            |   tok.Max_    [ _c = ValueRef::MAXIMUM ]
                        )
                        >>  ( '(' >>  expr [ push_back(_d, _1) ]
                        >>(*(',' >  expr [ push_back(_d, _1) ] )) >> ')' )
                        [ _val = construct_movable_(new_<ValueRef::Operation<T>>(
                            _c, deconstruct_movable_vector_(_d, _pass))) ]
                    )
                |   (
                    lit('(') >> expr [ push_back(_d, _1) ]
                    >> (
                        (       lit("==")   [ _c = ValueRef::COMPARE_EQUAL ]
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
                      ) [ _val = construct_movable_(new_<ValueRef::Operation<T>>(
                          _c, deconstruct_movable_vector_(_d, _pass))) ]
                )
                |   (
                    lit('-') >> functional_expr
                    // single parameter math function with a function expression
                    // rather than any arbitrary expression as parameter, because
                    // negating more general expressions can be ambiguous
                    [ _val = construct_movable_(new_<ValueRef::Operation<T>>(
                        ValueRef::NEGATE, deconstruct_movable_(_1, _pass))) ]
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
                 >> (       tok.Count_  [ _b = ValueRef::COUNT ]
                        |   tok.If_     [ _b = ValueRef::IF ]
                    )
                )
            >   label(tok.Condition_) > condition_parser
            [ _val = construct_movable_(new_<ValueRef::Statistic<T>>(
                deconstruct_movable_(_a, _pass), _b, deconstruct_movable_(_1, _pass))) ]
            ;

        statistic_value_expr
            =   (tok.Statistic_ >>      statistic_type_enum [ _b = _1 ])
            >   label(tok.Value_) >     statistic_value_ref_expr [ _a = _1 ]
            >   label(tok.Condition_) > condition_parser
            [ _val = construct_movable_(new_<ValueRef::Statistic<T>>(
                deconstruct_movable_(_a, _pass), _b, deconstruct_movable_(_1, _pass))) ]
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

    // Explicit instantiation to prevent costly recompilation in multiple units
    template arithmetic_rules<double>::arithmetic_rules(
        const std::string& type_name, const parse::lexer& tok,
        parse::detail::Labeller& label,
        const parse::detail::condition_parser_grammar& condition_parser);

    template arithmetic_rules<int>::arithmetic_rules(
        const std::string& type_name, const parse::lexer& tok,
        parse::detail::Labeller& label,
        const parse::detail::condition_parser_grammar& condition_parser);
} }
