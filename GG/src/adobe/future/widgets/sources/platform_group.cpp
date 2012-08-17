/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#include <GG/adobe/future/widgets/headers/platform_group.hpp>

#include <GG/adobe/future/widgets/headers/widget_utils.hpp>
#include <GG/adobe/future/widgets/headers/platform_label.hpp>
#include <GG/adobe/future/widgets/headers/platform_metrics.hpp>
#include <GG/adobe/future/widgets/headers/platform_widget_utils.hpp>
#include <GG/adobe/future/widgets/headers/display.hpp>

#include <GG/GroupBox.h>
#include <GG/GUI.h>
#include <GG/StyleFactory.h>


/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

group_t::group_t(const std::string& name,
                 const std::string& alt_text,
                 theme_t            theme) :
    control_m(0),
    name_m(name),
    alt_text_m(alt_text),
    theme_m(theme)
{ }

/****************************************************************************************************/

void group_t::measure(extents_t& result)
{
    assert(control_m);

    if (name_m.empty())
    {
        result.height() = 15;
        result.width() = 15;

        return;
    }

    // REVISIT (fbrereto) : A lot of static metrics values added here

    result = metrics::measure_text(name_m, implementation::DefaultFont());

    result.width() += 15;

    result.vertical().frame_m.first = result.height() + 7;

    result.height() = 5;
}

/****************************************************************************************************/

void group_t::place(const place_data_t& place_data)
{
    assert(control_m);
    implementation::set_control_bounds(control_m, place_data);
}

/****************************************************************************************************/

template <>
platform_display_type insert<group_t>(display_t&             display,
                                      platform_display_type& parent,
                                      group_t&               element)
{
    element.control_m =
        implementation::Factory().NewGroupBox(GG::X0, GG::Y0, GG::X1, GG::Y1, element.name_m,
                                              implementation::DefaultFont(), GG::CLR_GRAY);
    element.control_m->SetClientCornersEqualToBoxCorners(true);

    return display.insert(parent, element.control_m);
}

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/
