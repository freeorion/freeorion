/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_PARSER_SHARED_HPP
#define ADOBE_PARSER_SHARED_HPP

/*************************************************************************************************/

#include <GG/adobe/config.hpp>

#include <GG/adobe/istream_fwd.hpp>
#include <GG/adobe/name_fwd.hpp>

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/

void throw_parser_exception(const char*                     error_string,
                            const line_position_t&   position);

void throw_parser_exception(const char*                     expected,
                            const char*                     found,
                            const line_position_t&   position);

inline void throw_parser_exception( name_t                   expected,
                                    name_t                   found,
                                    const line_position_t&   position)
    { throw_parser_exception(expected.c_str(), found.c_str(), position); }

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/

#endif

/*************************************************************************************************/
