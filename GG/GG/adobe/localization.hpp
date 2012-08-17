/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_LOCALIZATION_HPP
#define ADOBE_LOCALIZATION_HPP

/*************************************************************************************************/

#include <GG/adobe/config.hpp>

#include <boost/function.hpp>

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/

/*!
\defgroup localization localization
\ingroup parsing
@{
*/

/*!
    The proc used here can expect utf-8 coming in and should output utf-8 encoded text in kind.
*/

typedef boost::function<std::string (const std::string&)> localization_lookup_proc_t;

void localization_register(const localization_lookup_proc_t& proc);

std::string localization_invoke(const std::string& source);

bool localization_ready();

//! @}

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/

#endif

/*************************************************************************************************/
