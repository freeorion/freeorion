/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#include <GG/adobe/future/number_formatter.hpp>
#include <GG/adobe/future/widgets/headers/display.hpp>
#include <GG/adobe/future/widgets/headers/widget_utils.hpp>
#include <GG/adobe/future/widgets/headers/platform_display_number.hpp>
#include <GG/adobe/future/widgets/headers/platform_metrics.hpp>

#include <GG/TextControl.h>
#include <GG/GUI.h>
#include <GG/StyleFactory.h>

#include <string>
#include <cassert>
#include <sstream>

/****************************************************************************************************/

namespace {

/****************************************************************************************************/

std::string set_field_text(std::string                       label,
                           double                            value,
                           const std::vector<adobe::unit_t>& unit_set)
{
    std::stringstream result;

    // this is called by a subview when its value changes.
    // we are only concerned with changing our actual value when
    // the view that changed is also the current view.

    std::string               suffix;
    adobe::number_formatter_t number_formatter;

    if (!unit_set.empty())
    {
        std::vector<adobe::unit_t>::const_iterator i(unit_set.begin());
        std::vector<adobe::unit_t>::const_iterator end(unit_set.end());

        while (i != end && i->scale_m_m <= value)
            ++i;

        if (i != unit_set.begin())
            --i;

        value = adobe::to_base_value(value, *i);

        suffix = i->name_m;

        number_formatter.set_format(i->format_m);
    }

    if (!label.empty())
        result << label << " ";

    result << number_formatter.format(value);

    if (!suffix.empty())
        result << " " << suffix;

    return result.str();
}

/****************************************************************************************************/

} // namespace

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

void display_number_t::place(const place_data_t& place_data)
{
    implementation::set_control_bounds(window_m, place_data);
}

/****************************************************************************************************/

void display_number_t::display(const model_type& value)
{
    assert(window_m);
    window_m->SetText(set_field_text(name_m, value, unit_set_m));
}

/****************************************************************************************************/

void display_number_t::measure(extents_t& result)
{
    assert(window_m);

    boost::shared_ptr<GG::Font> font = implementation::DefaultFont();

    extents_t space_extents(metrics::measure_text(std::string(" "), font));
    extents_t label_extents(metrics::measure_text(name_m, font));
    extents_t characters_extents(metrics::measure_text(std::string(characters_m, '0'), font));

    extents_t unit_max_extents;
    for (display_number_t::unit_set_t::iterator it(unit_set_m.begin()), end(unit_set_m.end());
         it != end;
         ++it)
    {
        extents_t tmp(metrics::measure_text(it->name_m, font));
        if (tmp.width() > unit_max_extents.width())
            unit_max_extents = tmp;
    }

    // set up default settings (baseline, etc.)
    result = space_extents;

    // set the width to the label width (if any)
    result.width() = label_extents.width();

    // add a guide for the label
    result.horizontal().guide_set_m.push_back(label_extents.width());

    // if there's a label, add space for a space
    if (label_extents.width() != 0)
        result.width() += space_extents.width();

    // append the character extents (if any)
    result.width() += characters_extents.width();

    // if there are character extents, add space for a space
    if (characters_extents.width() != 0)
        result.width() += space_extents.width();

    // append the max unit extents (if any)
    result.width() += unit_max_extents.width();

    assert(result.horizontal().length_m);
}

/****************************************************************************************************/

void display_number_t::measure_vertical(extents_t& calculated_horizontal, const place_data_t& placed_horizontal)
{
    assert(window_m);

    extents_t::slice_t& vert = calculated_horizontal.vertical();
    vert.length_m = Value(implementation::DefaultFont()->Lineskip());
    vert.guide_set_m.push_back(Value(implementation::DefaultFont()->Ascent()));
}

/****************************************************************************************************/

// REVISIT: MM--we need to replace the display_t mechanism with concepts/any_*/container idiom for event and drawing system.

template <>
platform_display_type insert<display_number_t>(display_t& display,
                                               platform_display_type& parent,
                                               display_number_t& element)
{
    element.window_m =
        implementation::Factory().NewTextControl(GG::X0, GG::Y0, GG::X1, GG::Y1,
                                                 element.name_m, implementation::DefaultFont());

    if (!element.alt_text_m.empty())
        implementation::set_control_alt_text(element.window_m, element.alt_text_m);

    return display.insert(parent, get_display(element));
}

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/
