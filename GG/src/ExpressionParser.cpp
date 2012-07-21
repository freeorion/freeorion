// -*- C++ -*-
/* GG is a GUI for SDL and OpenGL.
   Copyright (C) 2003-2008 T. Zachary Laine

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License
   as published by the Free Software Foundation; either version 2.1
   of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
    
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA

   If you do not wish to comply with the terms of the LGPL please
   contact the author as other terms are available for a fee.
    
   Zach Laine
   whatwasthataddress@gmail.com */
   
#include <GG/ExpressionParser.h>

#include <GG/adobe/implementation/token.hpp>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>


namespace {

    struct array_t_push_back_
    {
        template <typename Arg1, typename Arg2, typename Arg3 = void, typename Arg4 = void>
        struct result;

        template <typename Arg2, typename Arg3, typename Arg4>
        struct result<adobe::array_t, Arg2, Arg3, Arg4>
        { typedef void type; };

        template <typename Arg2>
        void operator()(adobe::array_t& array, Arg2 arg2) const
            { adobe::push_back(array, arg2); }

        template <typename Arg2, typename Arg3>
        void operator()(adobe::array_t& array, Arg2 arg2, Arg3 arg3) const
            {
                adobe::push_back(array, arg2);
                adobe::push_back(array, arg3);
            }

        template <typename Arg2, typename Arg3, typename Arg4>
        void operator()(adobe::array_t& array, Arg2 arg2, Arg3 arg3, Arg4 arg4) const
            {
                adobe::push_back(array, arg2);
                adobe::push_back(array, arg3);
                adobe::push_back(array, arg4);
            }
    };

    const boost::phoenix::function<array_t_push_back_> push;

    struct strip_quotes_
    {
        template <typename Arg>
        struct result
        { typedef std::string type; };

        std::string operator()(const std::string& arg1) const
            { return arg1.substr(1, arg1.size() - 2); }
    };

    const boost::phoenix::function<strip_quotes_> strip_quotes;

}

using namespace GG;

expression_parser_rules::expression_parser_rules(const lexer& tok, const keyword_rule& keyword_) :
    keyword(keyword_)
{
    namespace ascii = boost::spirit::ascii;
    namespace phoenix = boost::phoenix;
    namespace qi = boost::spirit::qi;
    using ascii::char_;
    using phoenix::clear;
    using phoenix::construct;
    using phoenix::if_;
    using phoenix::static_cast_;
    using phoenix::val;
    using qi::_1;
    using qi::_2;
    using qi::_3;
    using qi::_4;
    using qi::_a;
    using qi::_b;
    using qi::_r1;
    using qi::_val;
    using qi::alpha;
    using qi::bool_;
    using qi::digit;
    using qi::double_;
    using qi::eol;
    using qi::eps;
    using qi::lexeme;
    using qi::lit;

    expression
        =     or_expression(_r1)
        >>  - (
                   "?"
                >  expression(_a)
                >  ":"
                >  expression(_b)
              )
              [
                  push(_r1, _a, _b, adobe::ifelse_k)
              ]
        ;

    or_expression
        =     and_expression(_r1)
        >>  * (
                   tok.or_
                >  and_expression(_a)
              )
              [
                  push(_r1, _a, adobe::or_k),
                  clear(_a)
              ]
        ;

    and_expression
        =     equality_expression(_r1)
        >>  * (
                   tok.and_
                >  equality_expression(_a)
              )
              [
                  push(_r1, _a, adobe::and_k),
                  clear(_a)
              ]
        ;

    equality_expression
        =     relational_expression(_r1)
        >>  * (
                   tok.eq_op [_a = _1]
                >  relational_expression(_r1)
              )
              [
                  push(_r1, _a)
              ]
        ;

    relational_expression
        =     additive_expression(_r1)
        >>  * (
                   tok.rel_op [_a = _1]
                >  additive_expression(_r1)
              )
              [
                  push(_r1, _a)
              ]
        ;

    additive_expression
        =     multiplicative_expression(_r1)
        >>  * (
                   (
                        lit('+') [_a = adobe::add_k]
                     |  lit('-') [_a = adobe::subtract_k]
                   )
                >  multiplicative_expression(_r1)
              )
              [
                  push(_r1, _a)
              ]
        ;

    multiplicative_expression
        =     unary_expression(_r1)
        >>  * (
                   tok.mul_op [_a = _1]
                >  unary_expression(_r1)
              )
              [
                  push(_r1, _a)
              ]
        ;

    unary_expression
        =     postfix_expression(_r1)
        |     (
                   (
                        lit('+')
                     |  lit('-') [_a = adobe::unary_negate_k]
                     |  lit('!') [_a = adobe::not_k]
                   )
                >  unary_expression(_r1)
              )
              [
                  if_(_a) [push(_r1, _a)]
              ]
        ;

    // omitting unary_operator

    postfix_expression
        =     primary_expression(_r1)
        >>  * (
                   (
                        '['
                     >  expression(_r1)
                     >  ']'
                   )
                   [
                       push(_r1, adobe::bracket_index_k)
                   ]
                |  (
                        '.'
                     >  tok.identifier [push(_r1, _1)]
                   )
                   [
                       push(_r1, adobe::dot_index_k)
                   ]
              )
        ;

    primary_expression
        =     (
                   '('
                >> expression(_r1)
                >  ')'
              )
              [
                  push(_r1, adobe::parenthesized_expression_k)
              ]
        |     name(_r1)
        |     tok.number
              [
                  push(_r1, _1)
              ]
        |     boolean(_r1)
        |     string(_r1)
        |     tok.keyword_empty
              [
                  push(_r1, adobe::any_regular_t())
              ]
        |     array(_r1)
        |     dictionary(_r1)
        |     variable_or_function(_r1)
        ;

    variable_or_function
        =     (
                   tok.identifier [_a = _1]
                >> (
                        "("
                     >> (
                             argument_expression_list(_r1)
                          |  eps [push(_r1, adobe::array_t())]
                        )
                     >  ")"
                   )
              )
              [
                  push(_r1, _a, adobe::function_k)
              ]
        |     tok.identifier
              [
                  push(_r1, _1, adobe::variable_k)
              ]
        ;

    array
        =     '['
        >>    (
                   argument_list(_r1)
                |  eps [push(_r1, adobe::array_t())]
              )
        >     ']'
        ;

    dictionary
        =     '{'
        >>    (
                   named_argument_list(_r1)
                |  eps [push(_r1, adobe::dictionary_t())]
              )
        >     '}'
        ;

    argument_expression_list
        =     named_argument_list(_r1)
        |     argument_list(_r1)
        ;

    argument_list
        =     (
                    expression(_r1) [_a = 1]
               >> * (
                         ','
                      >  expression(_r1) [++_a]
                    )
              )
              [
                  push(_r1, static_cast_<double>(_a), adobe::array_k)
              ]
        ;

    named_argument_list
        =     (
                    named_argument(_r1) [_a = 1]
               >> * (
                         ','
                      >  named_argument(_r1) [++_a]
                    )
              )
              [
                  push(_r1, static_cast_<double>(_a), adobe::dictionary_k)
              ]
        ;

    named_argument
        =     tok.identifier [_a = _1]
        >>    lit(':') [push(_r1, _a)]
        >     expression(_r1)
        ;

    name
        =     '@'
        >     (
                   tok.identifier [push(_r1, _1)]
                |  keyword [push(_r1, _1)]
              )
              [
                  push(_r1, adobe::name_k)
              ]
        ;

    boolean = tok.keyword_true_false [push(_r1, _1)] ;


    // lexical grammar not covered by lexer

    string
        =     (
                     tok.quoted_string [_a = strip_quotes(_1)]
                >> * tok.quoted_string [_a += strip_quotes(_1)]
              )
              [
                  push(_r1, _a)
              ]
        ;

    keyword
        =     tok.keyword_true_false
              [
                  _val = if_else(_1, adobe::true_k, adobe::false_k)
              ]
        |     tok.keyword_empty
              [
                  _val = adobe::empty_k
              ]
        |     keyword_
              [
                  _val = _1
              ]
        ;


    // define names for rules, to be used in error reporting
#define NAME(x) x.name(#x)
    NAME(expression);
    NAME(or_expression);
    NAME(and_expression);
    NAME(equality_expression);
    NAME(relational_expression);
    NAME(additive_expression);
    NAME(multiplicative_expression);
    NAME(unary_expression);
    NAME(postfix_expression);
    NAME(primary_expression);
    NAME(variable_or_function);
    NAME(array);
    NAME(dictionary);
    NAME(argument_expression_list);
    NAME(argument_list);
    NAME(named_argument_list);
    NAME(named_argument);
    NAME(name);
    NAME(boolean);
    NAME(string);
#undef NAME

    qi::on_error<qi::fail>(expression, report_error(_1, _2, _3, _4));
}
