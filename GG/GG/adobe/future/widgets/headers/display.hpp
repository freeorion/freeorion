/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#ifndef ADOBE_DISPLAY_HPP
#define ADOBE_DISPLAY_HPP

/****************************************************************************************************/

#include <GG/adobe/config.hpp>

#include <GG/adobe/future/platform_primitives.hpp>

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

struct display_t
{
    display_t() :
        root_m(platform_display_type())
    { }

    void set_root(platform_display_type element)
        { root_m = element; }

    platform_display_type root()
        { return root_m; }

    platform_display_type insert(platform_display_type parent, platform_display_type element);

private:
    platform_display_type root_m;
};

/****************************************************************************************************/

display_t& get_main_display();

/****************************************************************************************************/

template <typename DisplayElement>
platform_display_type insert(display_t& display, platform_display_type& parent, DisplayElement& element);

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/

#endif // ADOBE_DISPLAY_HPP

/****************************************************************************************************/
