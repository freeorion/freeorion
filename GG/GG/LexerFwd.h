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
   
/** \file LexerFwd.h TODO. */

#ifndef _GG_LexerFwd_h_
#define _GG_LexerFwd_h_

#include <GG/adobe/name.hpp>

#include <boost/spirit/home/support/iterators/line_pos_iterator.hpp>
#include <boost/spirit/include/lex_lexertl.hpp>
#include <boost/spirit/home/lex/lexer/lexertl/position_token.hpp>

#include <string>


namespace GG {

typedef boost::spirit::line_pos_iterator<std::string::const_iterator> text_iterator;

typedef boost::spirit::lex::lexertl::position_token<
    text_iterator,
    boost::mpl::vector<
        adobe::name_t,
        std::string,
        double,
        bool
    >
> token_type;

typedef boost::spirit::lex::lexertl::actor_lexer<token_type> spirit_lexer_base_type;

}

#endif
