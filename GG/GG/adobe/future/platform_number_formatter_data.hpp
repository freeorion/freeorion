/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#ifndef ADOBE_NUMBER_FORMATTER_IMPL_HPP
#define ADOBE_NUMBER_FORMATTER_IMPL_HPP

/****************************************************************************************************/

#include <string>

#include <GG/adobe/any_regular.hpp>
#include <GG/adobe/dictionary.hpp>

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

struct number_formatter_platform_data_t
{
    number_formatter_platform_data_t()
    { initialize(); }

    void        initialize();
    void        set_format(const std::string& format);
    std::string get_format() const;
    std::string format(const any_regular_t& x);
    any_regular_t     parse(const std::string& str, any_regular_t the_type);
    void        monitor_locale(const dictionary_t& locale_data);

    std::string format_m;
};

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/

// ADOBE_NUMBER_FORMATTER_IMPL_HPP
#endif

/****************************************************************************************************/
