// -*- C++ -*-
#ifndef _Parse_h_
#define _Parse_h_

#ifndef _ValueRef_h_
#include <ValueRef.h>
#endif

#ifndef _GGEnum_h_
#include <GGEnum.h>
#endif

#ifndef SPIRIT_HPP
#include <boost/spirit.hpp>
#endif

#ifndef BOOST_SPIRIT_TREE_AST_HPP
#include <boost/spirit/tree/ast.hpp>
#endif

#include <stdexcept>

/** recognizes a known enumerated value.  The recognized value must be found in EnumMap<E> in order to be accepted.  This
    implies that an EnumMap must be defined for enum type E before the corresponding enum_parser is seen. */
template <class E>
struct enum_parser : public boost::spirit::parser<enum_parser<E> >
{
    typedef enum_parser self_t;

    template <typename ScannerT>
    struct result
    {
        typedef typename boost::spirit::match_result<ScannerT, E>::type type;
    };

    enum_parser () {}

    template <typename ScannerT>
    typename boost::spirit::parser_result<self_t, ScannerT>::type
    parse (ScannerT const &scan) const
    {
        using namespace boost::spirit;

        if (scan.at_end())
            return scan.no_match();

        typename ScannerT::iterator_t save = scan.first;

        std::string str;
        if ((alpha_p >> *(alnum_p | '_'))[assign(str)].parse(scan)) {
            E val;
            if ((val = E(GG::GetEnumMap<E>().FromString(str))) == E(GG::EnumMap<E>::BAD_VALUE)) {
                throw std::runtime_error("Encountered invalid enum value \"" + str + "\".");
            }
            return scan.create_match(str.length(), val, save, scan.first);
        }

        return scan.no_match();
    }
};

/** creates an AST (basically, a parse tree) from an arithmetic expression composed of elements of type T.  The types of 
    exressions recognized are basic arithmetic expressions involving +,-,*,/,-(unary) and paretheses.  Integer constants 
    and constants of type T are recognized (T may be a numeric type or an enum with a suitably defined EnumMap).  Finally,
    variables of the form Source.VarName or Target.VarName are recognized, where VarName is a symbol name recognized by 
    ValueRef::Variable. */
template <class T>
struct ArithmeticExpression : public boost::spirit::grammar<ArithmeticExpression>
{
    static const int ignore_ID =             1;
    static const int int_constant_ID =       2;
    static const int constant_ID =           3;
    static const int variable_ID =           4;
    static const int primary_expr_ID =       5;
    static const int negative_expr_ID =      6;
    static const int times_expr_ID =         7;
    static const int divides_expr_ID =       8;
    static const int plus_expr_ID =          9;
    static const int minus_expr_ID =        10;
    static const int expr_ID =              11;

    template <typename ScannerT>
    struct definition
    {
        definition(ArithmeticExpression const &)
        {
            using namespace boost::spirit;

            static enum_parser<T> enum_p;

            ignore = *(space_p | comment_p("/*", "*/"));

            int_constant = leaf_node_d[int_p];

            if (boost::is_enum<T>::value) {
                constant = leaf_node_d[enum_p];
            } else {
                if (boost::is_integral<T>::value) {
                    constant = leaf_node_d[int_p];
                } else {
                    constant = leaf_node_d[real_p];
                }
            }

            variable =
                leaf_node_d[(str_p("Source") | str_p("Target")) >> *('.' >> alpha_p >> *(alnum_p | '_'))];

            primary_expr =
                discard_node_d[ignore]
                >> (root_node_d[constant]
                   | root_node_d[int_constant]
                   | root_node_d[variable]
                   | inner_node_d['(' >> discard_node_d[ignore] >> expr >> discard_node_d[ignore] >> ')']);

            negative_expr =
                primary_expr
                | (root_node_d[ch_p('-')] >>
                  discard_node_d[ignore] >> primary_expr);

            times_expr =
                negative_expr >>
                *(discard_node_d[ignore] >> root_node_d[ch_p('*')] >>
                  discard_node_d[ignore] >> times_expr);

            divides_expr =
                times_expr >>
                *(discard_node_d[ignore] >> root_node_d[ch_p('/')] >>
                  discard_node_d[ignore] >> divides_expr);

            plus_expr =
                divides_expr >>
                *(discard_node_d[ignore] >> root_node_d[ch_p('+')] >>
                  discard_node_d[ignore] >> plus_expr);

            minus_expr =
                plus_expr >>
                *(discard_node_d[ignore] >> root_node_d[ch_p('-')] >>
                  discard_node_d[ignore] >> minus_expr);

            expr = minus_expr >> discard_node_d[ignore];
        }

        boost::spirit::rule<ScannerT,
                            boost::spirit::parser_context,
                            boost::spirit::parser_tag<ignore_ID> >        ignore;
        boost::spirit::rule<ScannerT,
                            boost::spirit::parser_context,
                            boost::spirit::parser_tag<constant_ID> >      constant;
        boost::spirit::rule<ScannerT,
                            boost::spirit::parser_context,
                            boost::spirit::parser_tag<int_constant_ID> >  int_constant;
        boost::spirit::rule<ScannerT,
                            boost::spirit::parser_context,
                            boost::spirit::parser_tag<variable_ID> >      variable;
        boost::spirit::rule<ScannerT,
                            boost::spirit::parser_context,
                            boost::spirit::parser_tag<primary_expr_ID> >  primary_expr;
        boost::spirit::rule<ScannerT,
                            boost::spirit::parser_context,
                            boost::spirit::parser_tag<negative_expr_ID> > negative_expr;
        boost::spirit::rule<ScannerT,
                            boost::spirit::parser_context,
                            boost::spirit::parser_tag<times_expr_ID> >    times_expr;
        boost::spirit::rule<ScannerT,
                            boost::spirit::parser_context,
                            boost::spirit::parser_tag<divides_expr_ID> >  divides_expr;
        boost::spirit::rule<ScannerT,
                            boost::spirit::parser_context,
                            boost::spirit::parser_tag<plus_expr_ID> >     plus_expr;
        boost::spirit::rule<ScannerT,
                            boost::spirit::parser_context,
                            boost::spirit::parser_tag<minus_expr_ID> >    minus_expr;
        boost::spirit::rule<ScannerT,
                            boost::spirit::parser_context,
                            boost::spirit::parser_tag<expr_ID> >          expr;

        const boost::spirit::rule<ScannerT,
                                boost::spirit::parser_context,
                                boost::spirit::parser_tag<expr_ID> > &
        start() const
        {
	        return expr;
        }
    };
};

/** takes the AST created by ArithmeticExpression<T>, and creates a ValueRef tree from it which can be evaluated later, once the 
    values of the variables are known. */
template <class T>
ValueRef::ValueRefBase<T>* EvalArithExpr(const boost::spirit::tree_match<char const*>::tree_iterator& it)
{
    typedef const boost::spirit::tree_match<char const*>::tree_iterator iter_t;

    using namespace ValueRef;

    if (it->value.id() == ArithmeticExpression<T>::constant_ID) {
        T const_value;
        std::string token(it->value.begin(), it->value.end());
        try {
            const_value = boost::lexical_cast<T>(token);
            return new Constant<T>(const_value);
        } catch (const boost::bad_lexical_cast&) {
            throw std::runtime_error("EvalIntArithExpr: No conversion exists from constant \"" + token + "\" to an acceptable value.");
        }
    } else if (it->value.id() == ArithmeticExpression<T>::int_constant_ID) {
        int int_const_value;
        std::string token(it->value.begin(), it->value.end());
        try {
            int_const_value = boost::lexical_cast<int>(token);
            return new Constant<T>(T(int_const_value));
        } catch (const boost::bad_lexical_cast&) {
            throw std::runtime_error("EvalIntArithExpr: No conversion exists for integer constant \"" + token + "\".");
        }
    } else if (it->value.id() == ArithmeticExpression<T>::variable_ID) {
        std::string token(it->value.begin(), it->value.end());
        return new Variable<T>(token.find("Source") != std::string::npos, token.substr(token.find('.') + 1));
    } else if (it->value.id() == ArithmeticExpression<T>::negative_expr_ID) {
        ValueRefBase<T> *arg = EvalArithExpr<T>(it->children.begin());
        return new Operation<T>(ValueRef::NEGATE, arg);
    } else if (it->value.id() == ArithmeticExpression<T>::times_expr_ID) {
        ValueRefBase<T> *lhs = EvalArithExpr<T>(it->children.begin());
        ValueRefBase<T> *rhs = EvalArithExpr<T>(it->children.begin() + 1);
        return new Operation<T>(ValueRef::TIMES, lhs, rhs);
    } else if (it->value.id() == ArithmeticExpression<T>::divides_expr_ID) {
        ValueRefBase<T> *lhs = EvalArithExpr<T>(it->children.begin());
        ValueRefBase<T> *rhs = EvalArithExpr<T>(it->children.begin() + 1);
        return new Operation<T>(ValueRef::DIVIDES, lhs, rhs);
    } else if (it->value.id() == ArithmeticExpression<T>::plus_expr_ID) {
        ValueRefBase<T> *lhs = EvalArithExpr<T>(it->children.begin());
        ValueRefBase<T> *rhs = EvalArithExpr<T>(it->children.begin() + 1);
        return new Operation<T>(ValueRef::PLUS, lhs, rhs);
    } else if (it->value.id() == ArithmeticExpression<T>::minus_expr_ID) {
        ValueRefBase<T> *lhs = EvalArithExpr<T>(it->children.begin());
        ValueRefBase<T> *rhs = EvalArithExpr<T>(it->children.begin() + 1);
        return new Operation<T>(ValueRef::MINUS, lhs, rhs);
    } else {
        throw std::runtime_error("EvalIntArithExpr: Invalid expression node value id.");
    }

    return 0;
}

/** creates an AST using ArithmeticExpression<T> from a string, and then uses EvalArithExpr<T> to create an ValueRef expression tree, all in one step. */
template <class T>
ValueRef::ValueRefBase<T>* ParseArithmeticExpression(const std::string& str)
{
    static ArithmeticExpression<T> arithmetic_expression;
	boost::spirit::tree_parse_info<> info = boost::spirit::ast_parse(str.c_str(), arithmetic_expression);
    return info.full ? EvalArithExpr<T>(info.trees.begin()) : 0;
}

#endif // _Parse_h_
