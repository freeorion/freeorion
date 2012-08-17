/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#include <GG/adobe/future/widgets/headers/platform_separator.hpp>
#include <GG/adobe/future/widgets/headers/display.hpp>

#include <GG/Control.h>


/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

separator_t::separator_t(bool    is_vertical,
                         theme_t theme) :
    control_m(0),
    is_vertical_m(is_vertical),
    theme_m(theme)
{ }

/*************************************************************************************************/

void separator_t::measure(extents_t& result)
{
#if 0 // TODO
#ifdef BOOST_MSVC
    result.horizontal().length_m   = 2;
    result.vertical().length_m     = 2;
#elif ADOBE_PLATFORM_MAC
    result.horizontal().length_m   = is_vertical_m ? 5 : 6;
    result.vertical().length_m     = is_vertical_m ? 6 : 5;
#endif
#endif
}

/*************************************************************************************************/

void separator_t::place(const place_data_t& place_data)
{ implementation::set_control_bounds(control_m, place_data); }

/****************************************************************************************************/

void separator_t::set_visible(bool make_visible)
{ set_control_visible(control_m, make_visible); }

/****************************************************************************************************/

template <>
platform_display_type insert<separator_t>(display_t&             display,
                                          platform_display_type& parent,
                                          separator_t&           element)
{
    assert(!element.control_m);

    assert(!"GG has no separator control yet!");
#if 0
    element.control_m = ::CreateWindowEx(WS_EX_COMPOSITED,
                                         WC_STATIC,
                                         NULL,
                                         WS_CHILD | WS_VISIBLE | SS_ETCHEDFRAME,
                                         0, 0, 10, 10,
                                         parent_hwnd,
                                         0,
                                         ::GetModuleHandle(NULL),
                                         NULL);
#endif

    return display.insert(parent, element.control_m);
}

/****************************************************************************************************/
}
