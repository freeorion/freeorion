/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_ISTREAM_FWD_HPP
#define ADOBE_ISTREAM_FWD_HPP

#include <GG/adobe/config.hpp>

#include <ios>

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/

struct line_position_t;

class stream_error_t;

std::string format_stream_error(std::istream&, const stream_error_t&);

std::string format_stream_error(const stream_error_t&);

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/

#endif // ADOBE_ISTREAM_FWD_HPP

/*************************************************************************************************/
