/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#include <GG/adobe/future/widgets/headers/platform_panel.hpp>

#include <GG/adobe/future/widgets/headers/display.hpp>
#include <GG/adobe/future/widgets/headers/platform_metrics.hpp>

#include <GG/Wnd.h>


/****************************************************************************************************/

namespace {

/****************************************************************************************************/

class PanelWnd :
    public GG::Wnd
{
public:
    PanelWnd() : Wnd(GG::X0, GG::Y0, GG::X1, GG::Y1, GG::Flags<GG::WndFlag>()) {}
};

/****************************************************************************************************/

} //end anonymous namespace

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

panel_t::panel_t(const any_regular_t& show_value, theme_t theme) :
    control_m(0),
    theme_m(theme),
    show_value_m(show_value)
{ }

/****************************************************************************************************/

void panel_t::display(const any_regular_t& value)
{
    bool visible(value == show_value_m);
    set_visible(visible);
}

/*************************************************************************************************/

void panel_t::measure(extents_t& result)
{ result = extents_t(); }

/*************************************************************************************************/

void panel_t::place(const place_data_t& place_data)
{ implementation::set_control_bounds(control_m, place_data); }

/*************************************************************************************************/

void panel_t::set_visible(bool make_visible)
{ set_control_visible(control_m, make_visible); }

/****************************************************************************************************/

template <>
platform_display_type insert<panel_t>(display_t&             display,
                                      platform_display_type& parent,
                                      panel_t&               element)
{
    element.control_m = new PanelWnd;
    return display.insert(parent, element.control_m);
}

/*************************************************************************************************/

} //namespace adobe

/*************************************************************************************************/
