/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#include <GG/adobe/future/widgets/headers/platform_toggle.hpp>
#include <GG/adobe/future/widgets/headers/button_helper.hpp>
#include <GG/adobe/future/widgets/headers/display.hpp>
#include <GG/adobe/future/widgets/headers/widget_utils.hpp>

#include <GG/Button.h>
#include <GG/GUI.h>
#include <GG/StyleFactory.h>


/****************************************************************************************************/

namespace {

/****************************************************************************************************/

void reset_textures(adobe::toggle_t& toggle)
{
    if (!toggle.control_m->Disabled()) {
        bool on = toggle.last_m == toggle.value_on_m;
        const GG::SubTexture& unpressed = on ? toggle.image_on_m : toggle.image_off_m;
        const GG::SubTexture& pressed = on ? toggle.image_off_m : toggle.image_on_m;
        toggle.control_m->SetUnpressedGraphic(unpressed);
        toggle.control_m->SetPressedGraphic(pressed);
        toggle.control_m->SetRolloverGraphic(unpressed);
    } else {
        toggle.control_m->SetUnpressedGraphic(toggle.image_disabled_m);
        toggle.control_m->SetPressedGraphic(toggle.image_disabled_m);
        toggle.control_m->SetRolloverGraphic(toggle.image_disabled_m);
    }
}

/****************************************************************************************************/

const GG::SubTexture& current_subtexture(adobe::toggle_t& toggle)
{
    if (toggle.control_m->Disabled()) {
        if (toggle.last_m == toggle.value_on_m)
            return toggle.image_on_m;
        else
            return toggle.image_off_m;
    } else { // disabled_button
        return toggle.image_disabled_m;
    }
}

/****************************************************************************************************/

void toggle_clicked(adobe::toggle_t& toggle)
{
    if (toggle.setter_proc_m.empty())
        return;

    // toggle it.
    adobe::any_regular_t new_value =
        toggle.last_m == toggle.value_on_m ?
        adobe::any_regular_t(adobe::empty_t()) :
        toggle.value_on_m;

    toggle.setter_proc_m(new_value);
}

/****************************************************************************************************/

} // namespace

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

toggle_t::toggle_t(const std::string&  alt_text,
                   const any_regular_t value_on,
                   const image_type&   image_on,
                   const image_type&   image_off,
                   const image_type&   image_disabled,
                   theme_t             theme) :
    control_m(0),
    theme_m(theme),
    alt_text_m(alt_text),
    image_on_m(
        image_on, GG::X0, GG::Y0,
        image_on->DefaultWidth(), image_on->DefaultHeight()
    ),
    image_off_m(
        image_off, GG::X0, GG::Y0,
        image_off->DefaultWidth(), image_off->DefaultHeight()
    ),
    image_disabled_m(
        image_disabled, GG::X0, GG::Y0,
        image_disabled->DefaultWidth(), image_disabled->DefaultHeight()
    ),
    value_on_m(value_on)
{ }

/****************************************************************************************************/

void toggle_t::measure(extents_t& result)
{
    assert(control_m);

    result = extents_t();

    const subtexture_type& image(current_subtexture(*this));

    result.height() = Value(image.Height());
    result.width() = Value(image.Width());
}

/****************************************************************************************************/

void toggle_t::place(const place_data_t& place_data)
{ implementation::set_control_bounds(control_m, place_data); }

/****************************************************************************************************/

void toggle_t::enable(bool make_enabled)
{
    assert(control_m);
    control_m->Disable(!make_enabled);
    reset_textures(*this);
}

/****************************************************************************************************/

void toggle_t::display(const any_regular_t& to_value)
{
    assert(control_m);

    if (last_m == to_value)
        return;

    last_m = to_value;
    reset_textures(*this);
}

/****************************************************************************************************/

void toggle_t::monitor(const setter_type& proc)
{
    assert(control_m);
    setter_proc_m = proc;
}

/****************************************************************************************************/

template <>
platform_display_type insert<toggle_t>(display_t&             display,
                                       platform_display_type& parent,
                                       toggle_t&              element)
{
    assert(!element.control_m);

    boost::shared_ptr<GG::StyleFactory> style = GG::GUI::GetGUI()->GetStyleFactory();
    element.control_m = style->NewButton(GG::X0, GG::Y0, GG::X(100), GG::Y(100),
                                         "", style->DefaultFont(), GG::CLR_GRAY);
    reset_textures(element);

    GG::Connect(element.control_m->ClickedSignal,
                boost::bind(&toggle_clicked, boost::ref(element)));

    if (!element.alt_text_m.empty())
        implementation::set_control_alt_text(element.control_m, element.alt_text_m);

    return display.insert(parent, element.control_m);
}

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/
