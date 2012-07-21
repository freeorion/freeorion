/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#ifndef ADOBE_SLIDER_HELPER_HPP
#define ADOBE_SLIDER_HELPER_HPP

/****************************************************************************************************/

#include <GG/adobe/future/widgets/headers/widget_tokens.hpp>
#include <GG/adobe/future/widgets/headers/value_range_format.hpp>

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

/*!
\brief Specifies the style of slider to be presented to the user
*/

enum slider_style_t
{
    /// Silder tab does not point in any direction
    slider_points_not_s = 0,

    /// Silder tab points up (horizontal sliders only)
    slider_points_up_s,

    /// Silder tab points down (horizontal sliders only)
    slider_points_down_s,

    /// Silder tab points left (vertical sliders only)
    slider_points_left_s,

    /// Silder tab points right (vertical sliders only)
    slider_points_right_s
};

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/

#endif

/****************************************************************************************************/
