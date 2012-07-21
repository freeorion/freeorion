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

#include <GG/Lexer.h>

#include <GG/adobe/name.hpp>


namespace GG {

    const boost::phoenix::function<report_error_<token_type> > report_error;

}

using namespace GG;

lexer::lexer(const adobe::name_t* first_keyword,
             const adobe::name_t* last_keyword) :
    keyword_true_false("true|false"),
    keyword_empty("empty"),
    identifier("[a-zA-Z]\\w*"),
    lead_comment("\\/\\*[^*]*\\*+([^/*][^*]*\\*+)*\\/"),
    trail_comment("\\/\\/.*$"),
    quoted_string("\\\"[^\\\"]*\\\"|'[^']*'"),
    number("\\d+(\\.\\d*)?"),
    eq_op("==|!="),
    rel_op("<|>|<=|>="),
    mul_op("\\*|\\/|%"),
    define("<=="),
    or_("\"||\""),
    and_("&&")
{
    namespace lex = boost::spirit::lex;

    self
        =     keyword_true_false
        |     keyword_empty;

    while (first_keyword != last_keyword) {
        self.add(
            keywords[*first_keyword] =
            boost::spirit::lex::token_def<adobe::name_t>(first_keyword->c_str())
        );
        ++first_keyword;
    }

    self
        +=    identifier
        |     lead_comment
        |     trail_comment
        |     quoted_string
        |     number
        |     eq_op
        |     rel_op
        |     mul_op
        |     define
        |     or_
        |     and_
        |     '+'
        |     '-'
        |     '!'
        |     '?'
        |     ':'
        |     '.'
        |     ','
        |     '('
        |     ')'
        |     '['
        |     ']'
        |     '{'
        |     '}'
        |     '@'
        |     ';'
        ;

    self("WS") = lex::token_def<>("\\s+");
}
