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
   
/** \file Lexer.h TODO. */

#ifndef _GG_Lexer_h_
#define _GG_Lexer_h_

#include <GG/LexerFwd.h>
#include <GG/ReportParseError.h>

#include <GG/adobe/istream.hpp>
#include <GG/adobe/implementation/token.hpp>


namespace GG {

namespace detail {
    struct named_eq_op : adobe::name_t {};
    struct named_rel_op : adobe::name_t {};
    struct named_mul_op : adobe::name_t {};
}

struct GG_API lexer :
    boost::spirit::lex::lexer<spirit_lexer_base_type>
{
    lexer(const adobe::name_t* first_keyword,
          const adobe::name_t* last_keyword);

    boost::spirit::lex::token_def<bool> keyword_true_false;
    boost::spirit::lex::token_def<boost::spirit::lex::omit> keyword_empty;
    boost::spirit::lex::token_def<adobe::name_t> identifier;
    boost::spirit::lex::token_def<std::string> lead_comment;
    boost::spirit::lex::token_def<std::string> trail_comment;
    boost::spirit::lex::token_def<std::string> quoted_string;
    boost::spirit::lex::token_def<double> number;
    boost::spirit::lex::token_def<detail::named_eq_op> eq_op;
    boost::spirit::lex::token_def<detail::named_rel_op> rel_op;
    boost::spirit::lex::token_def<detail::named_mul_op> mul_op;
    boost::spirit::lex::token_def<boost::spirit::lex::omit> define;
    boost::spirit::lex::token_def<boost::spirit::lex::omit> or_;
    boost::spirit::lex::token_def<boost::spirit::lex::omit> and_;
    std::map<adobe::name_t, boost::spirit::lex::token_def<adobe::name_t> > keywords;
};

typedef lexer::iterator_type token_iterator;

typedef lexer::lexer_def lexer_def;

typedef boost::spirit::qi::in_state_skipper<lexer_def> skipper_type;

extern const boost::phoenix::function<report_error_<token_type> > report_error;

}


// This code creates a new Spirit.Qi parser that does approximately what the
// Adobe lexer's next_position() function does.

namespace GG { namespace detail {
    BOOST_SPIRIT_TERMINAL(next_pos);
} }

namespace boost { namespace spirit {
    template <>
    struct use_terminal<qi::domain, GG::detail::tag::next_pos> :
        mpl::true_
    {};
} }

namespace GG { namespace detail {
    struct next_pos_parser :
        boost::spirit::qi::primitive_parser<next_pos_parser>
    {
        template <typename Context, typename Iter>
        struct attribute
        { typedef adobe::line_position_t type; };

        template <typename Iter, typename Context, typename Skipper, typename Attribute>
        bool parse(Iter& first, Iter const& last, Context&, Skipper const& skipper, Attribute& attr) const
        {
            boost::spirit::qi::skip_over(first, last, skipper);
            attr = adobe::line_position_t(detail::s_filename, boost::spirit::get_line(first->matched().begin()) - 1);
            // Note that the +1's below are there to provide the user with
            // 1-based column numbers.  This is Adobe's convention.  The Adobe
            // convention is also that line numbers are 0-based.  Go figure.
            attr.line_start_m =
                std::distance(detail::s_begin,
                              boost::spirit::get_line_start(detail::s_begin, first->matched().begin())) + 2;
            attr.position_m =
                std::distance(detail::s_begin, first->matched().begin()) + 1;
            return true;
        }

        template <typename Context>
        boost::spirit::info what(Context&) const
        { return boost::spirit::info("next_pos"); }
    };
} }

namespace boost { namespace spirit { namespace qi {
    template <typename Modifiers>
    struct make_primitive<GG::detail::tag::next_pos, Modifiers>
    {
        typedef GG::detail::next_pos_parser result_type;
        result_type operator()(unused_type, unused_type) const
        { return result_type(); }
    };
} } }


// These template specializations are required by Spirit.Lex to automatically
// convert an iterator pair to an adobe::name_t in detail::lexer.

namespace boost { namespace spirit { namespace traits
{
    // These template specializations are required by Spirit.Lex to automatically
    // convert an iterator pair to an adobe::name_t in the lexer below.

    template <typename Iter>
    struct assign_to_attribute_from_iterators<adobe::name_t, Iter>
    {
        static void call(const Iter& first, const Iter& last, adobe::name_t& attr)
            { attr = adobe::name_t(std::string(first, last).c_str()); }
    };

    // HACK! This is only necessary because of a bug in Spirit in Boost
    // versions <= 1.45.
    template <>
    struct GG_API assign_to_attribute_from_iterators<bool, GG::text_iterator, void>
    {
        static void call(const GG::text_iterator& first, const GG::text_iterator& last, bool& attr)
            { attr = *first == 't' ? true : false; }
    };

    template <typename Iter>
    struct assign_to_attribute_from_iterators<GG::detail::named_eq_op, Iter>
    {
        static void call(const Iter& first, const Iter& last, adobe::name_t& attr)
            { attr = *first == '=' ? adobe::equal_k : adobe::not_equal_k; }
    };

    template <typename Iter>
    struct assign_to_attribute_from_iterators<GG::detail::named_rel_op, Iter>
    {
        static void call(const Iter& first, const Iter& last, adobe::name_t& attr)
            {
                std::ptrdiff_t dist = std::distance(first, last);
                attr =
                    *first == '<' ?
                    (dist == 1 ? adobe::less_k : adobe::less_equal_k) :
                    (dist == 1 ? adobe::greater_k : adobe::greater_equal_k);
            }
    };

    template <typename Iter>
    struct assign_to_attribute_from_iterators<GG::detail::named_mul_op, Iter>
    {
        static void call(const Iter& first, const Iter& last, adobe::name_t& attr)
            {
                attr =
                    *first == '*' ?
                    adobe::multiply_k :
                    (*first == '/' ? adobe::divide_k : adobe::modulus_k);
            }
    };

} } }

#endif
