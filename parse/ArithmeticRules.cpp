#include "ValueRefParser.h"

#include "MovableEnvelope.h"
#include "../universe/ValueRefs.h"

#include <boost/phoenix.hpp>

namespace parse::detail {

    template <typename T>
    arithmetic_rules<T>::arithmetic_rules(
        const std::string& type_name,
        const parse::lexer& tok,
        parse::detail::Labeller& label,
        const parse::detail::condition_parser_grammar& condition_parser,
        const detail::value_ref_grammar<std::string>& string_grammar
    ) :
        statistic_type_enum(tok)
    {
        using boost::phoenix::construct;
        using boost::phoenix::new_;
        using boost::phoenix::push_back;

        boost::spirit::qi::_1_type _1;
        boost::spirit::qi::_2_type _2;
        boost::spirit::qi::_4_type _4;
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

        named_lookup_expr
          =   (
                   tok.Named_ >> tok.Value_ >> tok.Lookup_
                >> label(tok.name_)
                >> tok.string
              ) [ _val = construct_movable_(new_<ValueRef::NamedRef<T>>(_4, boost::phoenix::val(/*is_lookup_only*/true))) ]
            ;

        functional_expr
            =   (
                (
                    (
                            tok.sin_    [ _c = ValueRef::OpType::SINE ] // single-parameter math functions
                        |   tok.cos_    [ _c = ValueRef::OpType::COSINE ]
                        |   tok.log_    [ _c = ValueRef::OpType::LOGARITHM ]
                        |   tok.NoOp_   [ _c = ValueRef::OpType::NOOP ] // pass-through does nothing, for debug purposes
                        |   tok.abs_    [ _c = ValueRef::OpType::ABS ]
                        |   tok.round_  [ _c = ValueRef::OpType::ROUND_NEAREST ]
                        |   tok.ceil_   [ _c = ValueRef::OpType::ROUND_UP ]
                        |   tok.floor_  [ _c = ValueRef::OpType::ROUND_DOWN ]
                        |   tok.sign_   [ _c = ValueRef::OpType::SIGN ]
                    )
                    >> ('(' > expr > ')') [ _val = construct_movable_(new_<ValueRef::Operation<T>>(_c, deconstruct_movable_(_1, _pass))) ]
                )
                |   (
                    tok.RandomNumber_   [ _c = ValueRef::OpType::RANDOM_UNIFORM ] // random number requires a min and max value
                    >  ( '(' > expr >  ',' > expr > ')' ) [ _val = construct_movable_(
                            new_<ValueRef::Operation<T>>(_c, deconstruct_movable_(_1, _pass), deconstruct_movable_(_2, _pass))) ]
                )
                |   (
                    (
                            tok.OneOf_  [ _c = ValueRef::OpType::RANDOM_PICK ] // oneof, min, or max can take any number or operands
                        |   tok.min_    [ _c = ValueRef::OpType::MINIMUM ]
                        |   tok.max_    [ _c = ValueRef::OpType::MAXIMUM ]
                    )
                    >>  ( '(' >>  expr [ push_back(_d, _1) ]
                    >>(*(',' >  expr [ push_back(_d, _1) ] )) >> ')' )
                    [ _val = construct_movable_(new_<ValueRef::Operation<T>>(_c, deconstruct_movable_vector_(_d, _pass))) ]
                )
                |   (
                    lit('(') >> expr [ push_back(_d, _1) ]
                    >> (
                        (       lit("==")   [ _c = ValueRef::OpType::COMPARE_EQUAL ]
                              | lit('=')    [ _c = ValueRef::OpType::COMPARE_EQUAL ]
                              | lit(">=")   [ _c = ValueRef::OpType::COMPARE_GREATER_THAN_OR_EQUAL ]
                              | lit('>')    [ _c = ValueRef::OpType::COMPARE_GREATER_THAN ]
                              | lit("<=")   [ _c = ValueRef::OpType::COMPARE_LESS_THAN_OR_EQUAL ]
                              | lit('<')    [ _c = ValueRef::OpType::COMPARE_LESS_THAN ]
                              | lit("!=")   [ _c = ValueRef::OpType::COMPARE_NOT_EQUAL ]
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
                    [ _val = construct_movable_(new_<ValueRef::Operation<T>>(ValueRef::OpType::NEGATE, deconstruct_movable_(_1, _pass))) ]
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
                      >> functional_expr [ (
                          _b = construct_movable_(new_<ValueRef::Operation<T>>(
                              ValueRef::OpType::EXPONENTIATE,
                              deconstruct_movable_(_a, _pass),
                              deconstruct_movable_(_1, _pass) )),
                          _a = _b ) ]
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
                            lit('*') [ _c = ValueRef::OpType::TIMES ]
                        |   lit('/') [ _c = ValueRef::OpType::DIVIDE ]
                        |   lit('%') [ _c = ValueRef::OpType::REMAINDER ]
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
                            lit('+') [ _c = ValueRef::OpType::PLUS ]
                        |   lit('-') [ _c = ValueRef::OpType::MINUS ]
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
                 >> (   tok.Count_  [ _b = ValueRef::StatisticType::COUNT ]
                    |   tok.If_     [ _b = ValueRef::StatisticType::IF ]
                    )
                )
            >   label(tok.condition_) > condition_parser
            [ _val = construct_movable_(new_<ValueRef::Statistic<T>>(
                deconstruct_movable_(_a, _pass), _b, deconstruct_movable_(_1, _pass))) ]
            ;

        statistic_value_expr
            =  (tok.Statistic_ >> statistic_type_enum [ _b = _1 ])
            >>  label(tok.value_)
            >> (
                (
                        statistic_value_ref_expr [ _a = _1 ]
                    >   label(tok.condition_) > condition_parser
                    [ _val = construct_movable_(new_<ValueRef::Statistic<T, T>>(
                        deconstruct_movable_(_a, _pass), _b, deconstruct_movable_(_1, _pass))) ]
                )
            |   (
                        string_grammar [ _c = _1 ]
                    >   label(tok.condition_) > condition_parser
                    [ _val = construct_movable_(new_<ValueRef::Statistic<T, std::string>>(
                        deconstruct_movable_(_c, _pass), _b, deconstruct_movable_(_1, _pass))) ]
                )
               );

        statistic_expr
            =   statistic_collection_expr
            |   statistic_value_expr
            ;

        expr
            =   additive_expr
            ;

    #if DEBUG_VALUEREF_PARSERS
        debug(named_lookup_expr);
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

        named_lookup_expr.name(type_name + " nominal lookup expression");
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
        const std::string& type_name, const parse::lexer& tok, parse::detail::Labeller& label,
        const parse::detail::condition_parser_grammar& condition_parser,
        const detail::value_ref_grammar<std::string>& string_grammar);
    template arithmetic_rules<int>::arithmetic_rules(
        const std::string& type_name, const parse::lexer& tok, parse::detail::Labeller& label,
        const parse::detail::condition_parser_grammar& condition_parser,
        const detail::value_ref_grammar<std::string>& string_grammar);

}
