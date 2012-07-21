/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#ifndef ADOBE_WIDGET_WINDOW_HPP
#define ADOBE_WIDGET_WINDOW_HPP

/****************************************************************************************************/

#include <GG/Wnd.h>

#include <GG/adobe/config.hpp>

#include <GG/adobe/any_regular.hpp>
#include <GG/adobe/eve.hpp>
#include <GG/adobe/extents.hpp>
#include <GG/adobe/future/platform_primitives.hpp>
#include <GG/adobe/future/widgets/headers/window_helper.hpp>
#include <GG/adobe/layout_attributes.hpp>
#include <GG/adobe/widget_attributes.hpp>

#include <boost/function.hpp>

#include <string>


namespace GG {
    class Wnd;
}

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

struct window_t
{
    window_t(const std::string&     name,
             GG::Flags<GG::WndFlag> flags,
             theme_t                theme);

    ~window_t();

    void measure(extents_t& result);

    void place(const place_data_t& place_data);

    void set_visible(bool make_visible);

    void set_size(const point_2d_t& size);

    void reposition();
 
    void monitor_resize(const window_resize_proc_t& proc);

    any_regular_t underlying_handler();

    bool handle_key(key_type key, bool pressed, modifiers_t modifiers);

    GG::Wnd*               window_m;
    GG::Flags<GG::WndFlag> flags_m;
    std::string            name_m;
    theme_t                theme_m;
    place_data_t           place_data_m;
    window_resize_proc_t   resize_proc_m;
    bool                   debounce_m;
    bool                   placed_once_m;
};

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/

#endif

/****************************************************************************************************/
