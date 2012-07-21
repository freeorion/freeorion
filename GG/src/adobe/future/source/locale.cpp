/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#include <GG/adobe/future/locale.hpp>

#include <GG/adobe/future/platform_locale_data.hpp>

#include <GG/adobe/future/periodical.hpp>
#include <GG/adobe/name.hpp>
#include <GG/adobe/once.hpp>

/****************************************************************************************************/

ADOBE_ONCE_DECLARATION(locale_once)

/*************************************************************************************************/

namespace {

/*************************************************************************************************/

typedef boost::signal<void (const adobe::dictionary_t&)> locale_signal_type;

/*************************************************************************************************/

void init_locale_once()
{
    static adobe::periodical_t periodical(&adobe::implementation::do_locale_check, 1000);

    // get the current locale information as part of the initialization
    adobe::implementation::do_locale_check();
}

/*************************************************************************************************/

adobe::dictionary_t& current_locale_data()
{
    static adobe::dictionary_t locale_s;

    return locale_s;
}

locale_signal_type& locale_signal()
{
    static locale_signal_type signal_s;

    return signal_s;
}

/*************************************************************************************************/

} // namespace

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/

boost::signals::connection monitor_locale(const monitor_locale_proc_t& proc)
{
    ADOBE_ONCE_INSTANCE(locale_once);

    // signal the client right off the bat with the current locale information
    proc(current_locale_data());

    return locale_signal().connect(proc, boost::signals::at_back);
}

const dictionary_t& current_locale()
{
    ADOBE_ONCE_INSTANCE(locale_once);

    return current_locale_data();
}

/*************************************************************************************************/

namespace implementation {

/*************************************************************************************************/

void signal_locale_change(const dictionary_t& new_locale_data)
{
    current_locale_data() = new_locale_data;

    (locale_signal())(new_locale_data);
}

/*************************************************************************************************/

} // namespace implementation

/*************************************************************************************************/

aggregate_name_t key_locale_identifier                  = { "key_locale_identifier" };
aggregate_name_t key_locale_decimal_point       = { "key_locale_decimal_point" };
aggregate_name_t key_locale_thousands_separator = { "key_locale_thousands_separator" };

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/

ADOBE_ONCE_DEFINITION(locale_once, init_locale_once)

/*************************************************************************************************/
