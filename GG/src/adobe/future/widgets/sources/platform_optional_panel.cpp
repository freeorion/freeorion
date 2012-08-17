/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#include <GG/adobe/future/widgets/headers/platform_optional_panel.hpp>
#include <GG/adobe/future/widgets/headers/display.hpp>

#include <boost/bind.hpp>

/****************************************************************************************************/

namespace {

/****************************************************************************************************/

void dummy_display()
{ }

/****************************************************************************************************/

} // namespace

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

optional_panel_t::optional_panel_t(const any_regular_t& show_value,
                                   theme_t              theme) :
    control_m(show_value, theme),
    inited_m(false)
{ }

/*************************************************************************************************/

void optional_panel_t::display(const any_regular_t& value)
{
    assert(show_proc_m && hide_proc_m);

    bool visible (value == control_m.show_value_m);

    if (inited_m == false)
        control_m.set_visible(visible);

    boost::function<void ()> display_proc(boost::bind(&panel_t::set_visible, boost::ref(control_m), visible));
    boost::function<void ()> dummy_proc(&dummy_display);

    if (visible)
        show_proc_m(inited_m ? display_proc : dummy_display);
    else
        hide_proc_m(inited_m ? display_proc : dummy_display);

    inited_m = true;
}

/*************************************************************************************************/

void optional_panel_t::measure(extents_t& result)
{ result = extents_t(); }

/*************************************************************************************************/

void optional_panel_t::place(const place_data_t& place_data)
{ control_m.place(place_data); }

/****************************************************************************************************/

template <>
platform_display_type insert<optional_panel_t>(display_t&               display,
                                               platform_display_type&   parent,
                                               optional_panel_t& element)
{ return insert(display, parent, element.control_m); }

/*************************************************************************************************/

}

/*************************************************************************************************/
