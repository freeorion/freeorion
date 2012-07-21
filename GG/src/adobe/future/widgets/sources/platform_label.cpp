/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#include <GG/adobe/future/widgets/headers/platform_label.hpp>

#include <GG/adobe/future/widgets/headers/display.hpp>
#include <GG/adobe/future/widgets/headers/widget_utils.hpp>
#include <GG/adobe/future/widgets/headers/platform_metrics.hpp>
#include <GG/adobe/placeable_concept.hpp>

#include <string>
#include <cassert>

#include <GG/GUI.h>
#include <GG/StyleFactory.h>
#include <GG/TextControl.h>


/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

label_t::label_t(const std::string&        name,
                 const std::string&        alt_text,
                 std::size_t               characters,
                 GG::Flags<GG::TextFormat> format,
                 theme_t                   theme) :
    window_m(0),
    format_m(format),
    theme_m(theme),
    name_m(name),
    alt_text_m(alt_text),
    characters_m(characters)
{ }

/****************************************************************************************************/

void place(label_t& value, const place_data_t& place_data)
{
    implementation::set_control_bounds(value.window_m, place_data);
    if (!value.alt_text_m.empty())
        implementation::set_control_alt_text(value.window_m, value.alt_text_m);
}

/****************************************************************************************************/

void measure(label_t& value, extents_t& result)
{
    assert(value.window_m);
    GG::X w = static_cast<int>(value.characters_m) * implementation::CharWidth();
    GG::Pt extent = implementation::DefaultFont()->TextExtent(value.name_m, value.format_m, w);
    result.horizontal().length_m = Value(extent.x);
    assert(result.horizontal().length_m);
}

/****************************************************************************************************/

void measure_vertical(label_t& value, extents_t& calculated_horizontal, 
                      const place_data_t& placed_horizontal)
{
    assert(value.window_m);
    GG::Pt extent = implementation::DefaultFont()->TextExtent(value.name_m, value.format_m,
                                                              GG::X(width(placed_horizontal)));
    calculated_horizontal.vertical().length_m = Value(extent.y);
    calculated_horizontal.vertical().guide_set_m.push_back(
        Value(implementation::DefaultFont()->Ascent()));
}

/****************************************************************************************************/

void enable(label_t& value, bool make_enabled)
{ value.window_m->Disable(!make_enabled); }

/****************************************************************************************************/

extents_t measure_text(const std::string& text, const boost::shared_ptr<GG::Font>& font)
{
    extents_t result;
    GG::Pt size = implementation::DefaultFont()->TextExtent(text);
    result.horizontal().length_m = Value(size.x);
    result.vertical().length_m = Value(size.y);
    return result;
}

/****************************************************************************************************/

std::string get_control_string(const label_t& widget)
{ return widget.window_m->Text(); }

/****************************************************************************************************/

// REVISIT: MM--we need to replace the display_t mechanism with concepts/any_*/
//              container idiom for event and drawing system.

template <>
platform_display_type insert<label_t>(display_t& display,
                                      platform_display_type& parent,
                                      label_t& element)
{
    GG::X w = static_cast<int>(element.characters_m) * implementation::CharWidth();
    GG::Pt extent = implementation::DefaultFont()->TextExtent(element.name_m, element.format_m, w);
    element.window_m =
        implementation::Factory().NewTextControl(GG::X0, GG::Y0, extent.x, extent.y,
                                                 element.name_m, implementation::DefaultFont(),
                                                 GG::CLR_BLACK, element.format_m);

    if (!element.alt_text_m.empty())
        implementation::set_control_alt_text(element.window_m, element.alt_text_m);

    return display.insert(parent, get_display(element));
}


/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/
