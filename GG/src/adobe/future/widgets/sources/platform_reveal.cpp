/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#include <GG/adobe/future/widgets/headers/platform_reveal.hpp>

#include <GG/adobe/empty.hpp>
#include <GG/adobe/future/widgets/headers/display.hpp>
#include <GG/adobe/future/widgets/headers/widget_utils.hpp>
#include <GG/adobe/future/widgets/headers/platform_metrics.hpp>
#include <GG/adobe/placeable_concept.hpp>

#include <GG/Button.h>
#include <GG/GUI.h>
#include <GG/StyleFactory.h>


/****************************************************************************************************/

namespace {

/****************************************************************************************************/
GG::SubTexture showing_image()
{
    static boost::shared_ptr<GG::Texture> texture_s;
    if (!texture_s)
        texture_s = GG::GUI::GetGUI()->GetTexture("reveal_up.png");
    return GG::SubTexture(texture_s, GG::X0, GG::Y0, texture_s->Width(), texture_s->Height());
}

/****************************************************************************************************/

GG::SubTexture hidden_image()
{
    static boost::shared_ptr<GG::Texture> texture_s;
    if (!texture_s)
        texture_s = GG::GUI::GetGUI()->GetTexture("reveal_down.png");
    return GG::SubTexture(texture_s, GG::X0, GG::Y0, texture_s->Width(), texture_s->Height());
}

/****************************************************************************************************/

void reveal_clicked(adobe::reveal_t& reveal)
{
    if (reveal.hit_proc_m.empty())
        return;

    // toggle it.
    adobe::any_regular_t new_value =
        reveal.current_value_m == reveal.show_value_m ?
        adobe::any_regular_t(adobe::empty_t()) :
        reveal.show_value_m;

    reveal.hit_proc_m(new_value);
}

/****************************************************************************************************/

} // namespace

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

reveal_t::reveal_t(const std::string&   name,
                   const any_regular_t& show_value,
                   theme_t              theme,
                   const std::string&   alt_text) :
    control_m(0),
    theme_m(theme),
    name_m(name, std::string(), 0, GG::FORMAT_NONE, theme),
    using_label_m(!name.empty()),
    show_value_m(show_value),
    alt_text_m(alt_text)
{ }

/****************************************************************************************************/

void reveal_t::measure(extents_t& result)
{
    using adobe::measure;

    // REVISIT (fbrereto) : hardwired defaults
    result.width() = 16;
    result.height() = 17;

    if (!using_label_m)
        return;

    extents_t label_extents;

    measure(name_m, label_extents);

    place_data_t label_place;

    width(label_place) = label_extents.width();
    height(label_place) = label_extents.height();

    measure_vertical(name_m, label_extents, label_place);

    result.width() += 4 /* gap */ + label_extents.width();
    result.height() = (std::max)(result.height(), label_extents.height());
}

/****************************************************************************************************/

void reveal_t::place(const place_data_t& place_data)
{
    using adobe::place;

    assert(control_m);

    if (!using_label_m)
    {
        implementation::set_control_bounds(control_m, place_data);
    }
    else
    {
        place_data_t tmp(place_data);

        // REVISIT (fbrereto) : hardwired defaults
        width(tmp) = 16;
        height(tmp) = 17;

        implementation::set_control_bounds(control_m, tmp);

        width(tmp) = width(place_data) - 16 - 4 /* gap */;
        left(tmp) = left(place_data) + 16 + 4 /* gap */;

        place(name_m, tmp);
    }
}

/****************************************************************************************************/

void reveal_t::display(const any_regular_t& new_value)
{
    assert(control_m);

    if (current_value_m == new_value)
        return;

    current_value_m = new_value;

    const GG::SubTexture& subtexture =
        current_value_m == show_value_m ? showing_image() : hidden_image();
    control_m->SetUnpressedGraphic(subtexture);
    control_m->SetPressedGraphic(subtexture);
    control_m->SetRolloverGraphic(subtexture);
}

/****************************************************************************************************/

void reveal_t::monitor(const setter_type& proc)
{
    assert(control_m);
    hit_proc_m = proc;
}

/****************************************************************************************************/      

// REVISIT: MM--we need to replace the display_t mechanism with concepts/any_*/container idiom for event and drawing system.

template <>
platform_display_type insert<reveal_t>(display_t&             display,
                                       platform_display_type& parent,
                                       reveal_t&              element)
{
    assert(!element.control_m);

    boost::shared_ptr<GG::StyleFactory> style = GG::GUI::GetGUI()->GetStyleFactory();
    element.control_m = style->NewButton(GG::X0, GG::Y0, GG::X(100), GG::Y(100),
                                         "", style->DefaultFont(), GG::CLR_GRAY);

    const GG::SubTexture& subtexture =
        element.current_value_m == element.show_value_m ? showing_image() : hidden_image();
    element.control_m->SetUnpressedGraphic(subtexture);
    element.control_m->SetPressedGraphic(subtexture);
    element.control_m->SetRolloverGraphic(subtexture);

    GG::Connect(element.control_m->ClickedSignal,
                boost::bind(&reveal_clicked, boost::ref(element)));

    if (!element.alt_text_m.empty())
        implementation::set_control_alt_text(element.control_m, element.alt_text_m);

    platform_display_type result(display.insert(parent, element.control_m));

    if (element.using_label_m)
        display.insert(parent, get_display(element.name_m));

    return result;
}


/****************************************************************************************************/

} // namespace adobe
