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
   
/** \file ReportParseError.h TODO. */

#ifndef _GG_ReportParseError_h_
#define _GG_ReportParseError_h_

#include <GG/Export.h>
#include <GG/LexerFwd.h>

#include <boost/algorithm/string/replace.hpp>
#include <boost/tuple/tuple.hpp>


namespace GG {

    namespace detail {

        inline void default_send_error_string(const std::string& str)
        { std::cerr << str; }

        extern GG_API const char* s_filename;
        extern GG_API text_iterator* s_text_it;
        extern GG_API text_iterator s_begin;
        extern GG_API text_iterator s_end;

    }

    template <typename TokenType>
    struct report_error_
    {
        template <typename Arg1, typename Arg2, typename Arg3, typename Arg4>
        struct result
        { typedef void type; };

        template <typename Arg1, typename Arg2, typename Arg3, typename Arg4>
        void operator()(Arg1 first, Arg2, Arg3 it, Arg4 rule_name) const
            {
                std::string error_string;
                generate_error_string(first, it, rule_name, error_string);
                send_error_string(error_string);
            }

        static boost::function<void (const std::string&)> send_error_string;

    private:
        std::pair<text_iterator, unsigned int> line_start_and_line_number(text_iterator error_position) const
            {
                unsigned int line = 1;
                text_iterator it = detail::s_begin;
                text_iterator line_start = detail::s_begin;
                while (it != error_position) {
                    bool eol = false;
                    if (it != error_position && *it == '\r') {
                        eol = true;
                        line_start = ++it;
                    }
                    if (it != error_position && *it == '\n') {
                        eol = true;
                        line_start = ++it;
                    }
                    if (eol)
                        ++line;
                    else
                        ++it;
                }
                return std::pair<text_iterator, unsigned int>(line_start, line);
            }

        std::string get_line(text_iterator line_start) const
            {
                text_iterator line_end = line_start;
                while (line_end != detail::s_end && *line_end != '\r' && *line_end != '\n') {
                    ++line_end;
                }
                return std::string(line_start, line_end);
            }

        template <typename TokenIter>
        void generate_error_string(const TokenIter& first,
                                   const TokenIter& it,
                                   const boost::spirit::info& rule_name,
                                   std::string& str) const
            {
                std::stringstream is;

                GG::text_iterator line_start;
                unsigned int line_number;
                GG::text_iterator text_it = it->matched().begin();
                if (it->matched().begin() == it->matched().end()) {
                    text_it = *detail::s_text_it;
                    if (text_it != detail::s_end)
                        ++text_it;
                }
                boost::tie(line_start, line_number) = line_start_and_line_number(text_it);
                std::size_t column_number = std::distance(line_start, text_it);

                is << detail::s_filename << ":" << line_number << ":" << column_number << ": "
                   << "Parse error: expected " << rule_name;

                if (text_it == detail::s_end) {
                    is << " before end of input.\n";
                } else {
                    is << " here:\n"
                       << "  " << get_line(line_start) << "\n"
                       << "  " << std::string(column_number, ' ') << '^' << std::endl;
                }

                str = is.str();
            }
    };

    template <typename TokenType>
    boost::function<void (const std::string&)> report_error_<TokenType>::send_error_string =
        &detail::default_send_error_string;

}

#endif
