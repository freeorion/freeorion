/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_FUTURE_LOCALE_HPP
#define ADOBE_FUTURE_LOCALE_HPP

/*************************************************************************************************/

#include <GG/adobe/config.hpp>

#include <GG/adobe/dictionary.hpp>

#include <boost/function.hpp>
#include <boost/signal.hpp>

#include <GG/adobe/future/platform_locale_data.hpp>

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/

typedef boost::function<void (const dictionary_t& locale_data)> monitor_locale_proc_t;

boost::signals::connection monitor_locale(const monitor_locale_proc_t& proc);

const dictionary_t& current_locale();

/*************************************************************************************************/

extern aggregate_name_t key_locale_identifier;
extern aggregate_name_t key_locale_decimal_point;
extern aggregate_name_t key_locale_thousands_separator;

/*************************************************************************************************/

namespace implementation {

/*************************************************************************************************/

void signal_locale_change(const dictionary_t& new_locale_data);

/*************************************************************************************************/

} // namespace implementation

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/

#endif

/*************************************************************************************************/
