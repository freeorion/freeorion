/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#include <GG/adobe/future/widgets/headers/button_helper.hpp>
#include <GG/adobe/future/widgets/headers/display.hpp>
#include <GG/adobe/future/widgets/headers/platform_button.hpp>
#include <GG/adobe/future/widgets/headers/platform_label.hpp>
#include <GG/adobe/future/widgets/headers/platform_metrics.hpp>
#include <GG/adobe/future/widgets/headers/platform_widget_utils.hpp>

#include <GG/Button.h>
#include <GG/GUI.h>
#include <GG/StyleFactory.h>

/****************************************************************************************************/

namespace {

struct Clicked
{
    Clicked(adobe::button_t& button) :
        m_button(button)
        {}
    void operator()()
        {
            adobe::button_state_set_t::iterator state(
                adobe::button_modifier_state(m_button.state_set_m,
                                             m_button.modifier_mask_m,
                                             m_button.modifiers_m));

            if (state == m_button.state_set_m.end())
                state = adobe::button_default_state(m_button.state_set_m);

            if (!state->hit_proc_m.empty())
                state->hit_proc_m(state->value_m, state->contributing_m);
        }
    adobe::button_t& m_button;
};

}

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

button_t::button_t(bool                             is_default,
                   bool                             is_cancel,
                   modifiers_t                      modifier_mask,
                   const button_state_descriptor_t* first,
                   const button_state_descriptor_t* last,
                   theme_t                          theme) :
    control_m(0),
    theme_m(theme),
    state_set_m(first, last),
    modifier_mask_m(modifier_mask),
    modifiers_m(modifiers_none_s),
    is_default_m(is_default),
    is_cancel_m(is_cancel),
    enabled_m(true)
{ }

/****************************************************************************************************/

void button_t::measure(extents_t& result)
{
    result = metrics::measure(control_m);

    button_state_set_t::iterator state(button_modifier_state(state_set_m,
                                                             modifier_mask_m,
                                                             modifiers_m));

    boost::shared_ptr<GG::Font> font = implementation::DefaultFont();

    if (state == state_set_m.end())
        state = button_default_state(state_set_m);

    extents_t cur_text_extents(metrics::measure_text(state->name_m, font));

    result.width() -= cur_text_extents.width();
    result.height() = Value(control_m->Height());

    long width_additional(0);
    
    for (button_state_set_t::iterator iter(state_set_m.begin()), last(state_set_m.end()); iter != last; ++iter)
    {
        extents_t tmp(metrics::measure_text(iter->name_m, font));
        width_additional = (std::max)(width_additional, tmp.width());
    }

    result.width() += width_additional;
    result.width() += Value(2 * implementation::CharWidth());

    result.width() = (std::max)(result.width(), 70L);
}

/****************************************************************************************************/

void button_t::place(const place_data_t& place_data)
{
    assert(control_m);
    implementation::set_control_bounds(control_m, place_data);
}

/****************************************************************************************************/

void button_t::enable(bool make_enabled)
{
    enabled_m = make_enabled;
    if (control_m)
        control_m->Disable(!make_enabled);
}

/****************************************************************************************************/

void button_t::set(modifiers_t modifiers, const model_type& value)
{
    button_state_set_t::iterator state(button_modifier_state(state_set_m, modifier_mask_m, modifiers));

    if (state == state_set_m.end())
        state = button_default_state(state_set_m);

    if (state->value_m != value)
        state->value_m = value;
}

/****************************************************************************************************/

void button_t::set_contributing(modifiers_t modifiers, const dictionary_t& value)
{
    button_state_set_t::iterator state(button_modifier_state(state_set_m, modifier_mask_m, modifiers));

    if (state == state_set_m.end())
        state = button_default_state(state_set_m);

    state->contributing_m = value;
}

/****************************************************************************************************/

bool button_t::handle_key(key_type key, bool pressed, modifiers_t /* modifiers */)
{
    if (pressed == false)
        return false;

    modifiers_m = modifier_state();

    //
    // Look up the state which this modifier should trigger.
    //
    button_state_set_t::iterator state(button_modifier_state(state_set_m,
                                                             modifier_mask_m,
                                                             modifiers_m));

    if (state == state_set_m.end())
        state = button_default_state(state_set_m);

    //
    // Set the window text.
    //
    control_m->SetText(state->name_m);

    //
    // Set the alt text if need be.
    //
    if (!state->alt_text_m.empty())
        implementation::set_control_alt_text(control_m, state->alt_text_m);

    if (state->hit_proc_m.empty() || enabled_m == false)
        return false;

    if ((key.first == GG::GGK_RETURN || key.first == GG::GGK_KP_ENTER) && is_default_m)
        state->hit_proc_m(state->value_m, state->contributing_m);
    else if (key.first == GG::GGK_ESCAPE && is_cancel_m)
        state->hit_proc_m(state->value_m, state->contributing_m);
    else
        return false;

    return true;
}

/****************************************************************************************************/

template <>
platform_display_type insert<button_t>(display_t&             display,
                                       platform_display_type& parent,
                                       button_t&              element)
{
    assert(element.control_m == 0);

    button_state_set_t::iterator state(button_default_state(element.state_set_m));

    element.control_m =
        implementation::Factory().NewButton(GG::X0, GG::Y0, GG::X1, implementation::StandardHeight(),
                                            state->name_m, implementation::DefaultFont(), GG::CLR_GRAY);

    GG::Connect(element.control_m->ClickedSignal, Clicked(element));

    if (!state->alt_text_m.empty())
        implementation::set_control_alt_text(element.control_m, state->alt_text_m);

    element.control_m->Disable(!element.enabled_m);

    return display.insert(parent, element.control_m);
}

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/
