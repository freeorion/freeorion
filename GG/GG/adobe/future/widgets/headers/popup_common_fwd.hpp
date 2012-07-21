/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#ifndef ADOBE_POPUP_COMMON_FWD_HPP
#define ADOBE_POPUP_COMMON_FWD_HPP

/****************************************************************************************************/

#include <boost/function.hpp>

#include <GG/adobe/any_regular.hpp>
#include <GG/adobe/widget_attributes.hpp>

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

typedef boost::function<void (const any_regular_t&)>              popup_setter_type;
typedef boost::function<void (const any_regular_t&, modifiers_t)> popup_extended_setter_type;

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/

// ADOBE_POPUP_COMMON_HPP
#endif

/****************************************************************************************************/
