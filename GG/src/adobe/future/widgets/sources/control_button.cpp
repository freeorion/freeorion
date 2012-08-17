/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#define ADOBE_DLL_SAFE 0

#include <GG/adobe/config.hpp>

#include <GG/adobe/future/widgets/headers/control_button.hpp>
#include <GG/adobe/future/widgets/headers/button_helper.hpp>

#include <iomanip>
#include <limits>

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/

control_button_t::control_button_t(const std::string&          name,
                                   const std::string&          alt_text,
                                   const expression_eval_proc& eval_proc,
                                   const array_t&              expression,
                                   theme_t                     theme) :
    eval_proc_m(eval_proc),
    expression_m(expression)
{
    button_state_descriptor_t        state;
    const button_state_descriptor_t* first(&state);

    state.name_m = name;
    state.alt_text_m = alt_text;
    state.hit_proc_m = boost::bind(&control_button_t::button_fire, boost::ref(*this), _1, _2);

    button_m.reset(new button_t(false, false, modifiers_t(), first, boost::next(first), theme));
}

/*************************************************************************************************/

void control_button_t::button_fire(const any_regular_t&, const dictionary_t&)
{
    if (proc_m && eval_proc_m)
        proc_m(eval_proc_m(expression_m));
}

/*************************************************************************************************/

void control_button_t::measure(extents_t& result)
{
    assert(button_m.get());

    button_m->measure(result);
}

/*************************************************************************************************/

void control_button_t::place(const place_data_t& place_data)
{
    assert(button_m.get());

    button_m->place(place_data);
}

/*************************************************************************************************/

void control_button_t::monitor(const setter_proc_type& proc)
{
    proc_m = proc;
}

/*************************************************************************************************/

void control_button_t::enable(bool make_enabled)
{
    assert(button_m.get());

    button_m->enable(make_enabled);
}

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/
