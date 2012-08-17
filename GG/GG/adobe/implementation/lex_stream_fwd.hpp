/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_LEX_STREAM_FWD_HPP
#define ADOBE_LEX_STREAM_FWD_HPP

/*************************************************************************************************/

#include <GG/adobe/config.hpp>

#ifdef __MWERKS__
    #pragma warn_implicitconv off
#endif

#include <GG/adobe/implementation/lex_shared_fwd.hpp>

#ifdef __MWERKS__
    #pragma warn_implicitconv reset
#endif

#include <boost/function.hpp>

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/

typedef bool (keyword_extension_lookup_proc_signature_t)(const name_t&);
typedef boost::function<keyword_extension_lookup_proc_signature_t> keyword_extension_lookup_proc_t;

/*************************************************************************************************/

class lex_stream_t;

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/

#endif

/*************************************************************************************************/
