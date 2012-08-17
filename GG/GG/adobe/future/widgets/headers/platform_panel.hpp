/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#ifndef ADOBE_PANEL_HPP
#define ADOBE_PANEL_HPP

#include <GG/adobe/config.hpp>

#include <boost/utility.hpp>

#include <GG/adobe/any_regular.hpp>
#include <GG/adobe/extents.hpp>
#include <GG/adobe/layout_attributes.hpp>
#include <GG/adobe/widget_attributes.hpp>
#include <GG/adobe/future/widgets/headers/widget_utils.hpp>


namespace GG {
    class Wnd;
}

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

struct panel_t : extents_slices_t, boost::noncopyable
{
    typedef any_regular_t model_type;

    panel_t(const any_regular_t& show_value, theme_t theme);

    void        measure(extents_t& result);

    void        place(const place_data_t& place_data);

    void        display(const any_regular_t& value);

    void        set_visible(bool make_visible); 

    GG::Wnd*         control_m;
    theme_t          theme_m;
    any_regular_t    show_value_m;
};

/****************************************************************************************************/

}

/****************************************************************************************************/

#endif

/*************************************************************************************************/
