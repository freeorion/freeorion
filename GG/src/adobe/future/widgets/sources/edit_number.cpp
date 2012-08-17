/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#define ADOBE_DLL_SAFE 0

#include <GG/Edit.h>

#include <GG/adobe/config.hpp>

#include <GG/adobe/algorithm/find.hpp>
#include <GG/adobe/poly_placeable.hpp>
#include <GG/adobe/future/widgets/headers/widget_utils.hpp>
#include <GG/adobe/name.hpp>
#include <GG/adobe/future/widgets/headers/edit_number.hpp>
#include <GG/adobe/future/widgets/headers/display.hpp>

#include <iomanip>
#include <limits>

/*************************************************************************************************/

namespace {

/*************************************************************************************************/

inline adobe::edit_number_t::base_unit_set_t::iterator 
find(adobe::edit_number_t::base_unit_set_t& set, adobe::name_t name)
{
    return adobe::find_if(set, 
                          boost::bind(adobe::compare_members(
                                          &adobe::edit_number_t::base_unit_t::name_m,
                                          std::equal_to<adobe::name_t>()),
                                      _1, name));
}

/*************************************************************************************************/

template <typename ForwardIterator>
adobe::edit_number_t::base_unit_set_t base_unit_set(ForwardIterator first, ForwardIterator last)
{
    adobe::edit_number_t::base_unit_set_t result;

    for (; first != last; ++first)
    {
        adobe::edit_number_t::base_unit_set_t::iterator base_unit_iter(
            find(result, first->base_unit_m));

        if (base_unit_iter == result.end())
        {
            result.push_back(adobe::edit_number_t::base_unit_t(first->base_unit_m,
                                                               first->min_value_m,
                                                               first->max_value_m));
        }
        else
        {
            // set the minimum value to the min (valid) value (::min() is considered not valid)
            if (first->min_value_m != (std::numeric_limits<double>::min)())
            {
                base_unit_iter->min_m =
                    base_unit_iter->min_m == (std::numeric_limits<double>::min)() ?
                        first->min_value_m :
                        (std::min)(base_unit_iter->min_m, first->min_value_m);
            }

            // set the maximum value to the max (valid) value (::max() is considered not valid)
            if (first->max_value_m != (std::numeric_limits<double>::max)())
            {
                base_unit_iter->max_m =
                    base_unit_iter->max_m == (std::numeric_limits<double>::max)() ?
                        first->max_value_m :
                        (std::max)(base_unit_iter->max_m, first->max_value_m);
            }
        }
    }

    return result;
}

/*************************************************************************************************/

bool is_intermediate_state(const std::string& candidate)
{
    static const std::string empty("");
    static const std::string minus("-");

    std::string decimal_point;

    get_value(adobe::current_locale(), adobe::key_locale_decimal_point, decimal_point);

    // If the text is one of several "unsafe", presumably the user is about
    // to enter in more information, so we allow then. The value should go to 0.

    return candidate == empty ||
           candidate == minus ||
           (!decimal_point.empty() && candidate == decimal_point);
}

/*************************************************************************************************/

} // namespace

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/

/*************************************************************************************************/

const adobe_cursor_t edit_number_t::scrubby_cursor()
{
    static adobe_cursor_t scrubby_cursor;
    static bool           inited(false);

    // make_cursor has the potential to throw, so we are
    // a little more guarded about constructing the
    // cursor than we usually are to account for this.
    // if we fail to load it the first time, we do not
    // try again.

    if (!inited)
    {
        inited = true;
        scrubby_cursor = make_cursor("cursor_scrub_slider.tga", 15, 15);
    }

    return scrubby_cursor;
}

/*************************************************************************************************/

bool edit_number_t::handle_key(key_type key, bool pressed, modifiers_t /*modifiers*/)
{
    if (!implementation::is_focused(edit_text_m.control_m) || !pressed)
        return false;

    bool handled(true);

    if (key.first == GG::GGK_UP)
        increment(true);
    else if (key.first == GG::GGK_DOWN)
        increment(false);
    else
        handled = false;

    return handled;
}

/*************************************************************************************************/

void edit_number_t::measure(extents_t& result)
{
    // REVISIT (fbrereto) : This whole measure/place suite needs better
    //                      handling of the baseline alignment issue; we
    //                      skirt it currently and we should not.

    using adobe::measure;

    measure(edit_text_m, result);

    if (!using_popup())
        return; 

    edit_text_width_m = result.width();

    extents_t popup_extents;

    measure(popup_m, popup_extents);

    // REVISIT (fbrereto) : constant value
    result.width() += 4 /*gap*/ + popup_extents.width();
    result.height() = (std::max)(result.height(), popup_extents.height());
}

/*************************************************************************************************/

void edit_number_t::place(const place_data_t& place_data)
{
    using adobe::place;

    place_data_t edit_place(place_data);
    place_data_t popup_place(place_data);

    if (using_popup())
        width(edit_place) = static_cast<long>(edit_text_width_m);

    place(edit_text_m, edit_place);

    if (!using_popup())
        return;

    // REVISIT (fbrereto) : constant value
    long offset(static_cast<long>(edit_text_width_m + 4 /*gap*/));

    popup_place.horizontal().guide_set_m.clear();
    width(popup_place) -= offset;
    left(popup_place)  += offset;

    place(popup_m, popup_place);
}

/*************************************************************************************************/

void edit_number_t::monitor_text(const std::string& new_value, bool display_was_updated)
{
    // Update the value of the model to the new value

    unit_t&          unit(unit_set_m[unit_index_m]);
    std::size_t      base_unit_index(current_base_unit_index());
    double           real(number_formatter_m.parse<model_type>(new_value));
    controller_type& controller(controller_set_m[base_unit_index]);
    debounce_type&   debounce(debounce_set_m[base_unit_index]);

    if (is_intermediate_state(new_value))
    {
        real = 0;

        // we set display_was_updated here so the user isn't rudely
        // given a '0' in the edit text field they are editing.

        display_was_updated = true;
    }
    else
    {
        // reverse-scale the value to the base units    

        real = to_pinned_base_value(real, unit,
                                    base_unit_set_m[base_unit_index].min_m,
                                    base_unit_set_m[base_unit_index].max_m);
    }

    if (debounce == real)
        return;

    if (display_was_updated)
        debounce = real;

    if (controller.setter_m)
        controller.setter_m(real);
}

/*************************************************************************************************/

void edit_number_t::display_unit(const edit_number_unit_subwidget_t::model_type& new_value)
{
    // When the user changes the popup value we're going to do one of two things
    // predicated on whether or not the edit_text field currently has the focus.
    // If edit_text has the focus, then the user is setting the unit for the value
    // they just entered -- we want to change to that index and set the new value
    // accordingly. If however edit_text does not have the focus, then we are just
    // altering the value that is being shown and we're not going to send anything
    // to the model.

    unit_set_t::iterator iter =
        adobe::find_if(unit_set_m, boost::bind(adobe::compare_members(
                                                   &unit_set_t::value_type::name_m,
                                                   std::equal_to<std::string>()),
                                               _1, new_value));

    if (iter == unit_set_m.end())
        return;

    unit_index_m = std::size_t(std::distance(unit_set_m.begin(), iter));

    std::size_t base_unit_index(current_base_unit_index());
    refresh_enabled_state(&controller_set_m[base_unit_index]);

    // check for edit_text focus and do the right thing.
    if (implementation::is_focused(edit_text_m.control_m) && 
        controller_set_m[base_unit_index].enabled_m)
    {
        // reset the value of the new unit to the scale of the new value
        // as it can be extracted from the edit_text field at this point.

        monitor_text(implementation::get_field_text(edit_text_m));
    }
    else
    {
        // not touching the model; just change up what's being shown
        refresh_view(&view_set_m[base_unit_index], debounce_set_m[base_unit_index], true);
    }
}

/*************************************************************************************************/

void edit_number_t::field_text_filter(const std::string& candidate, bool& squelch)
{
    if (is_intermediate_state(candidate))
        return;

    if (candidate.find(30) != std::string::npos) // up key
        increment(true);
    else if (candidate.find(31) != std::string::npos) // down key
        increment(false);
    else
    {
        squelch = !completely_valid_number_string_given_current_locale(candidate);

        if (squelch == false)
        {
            double      real(number_formatter_m.parse<model_type>(candidate));
            unit_t&     unit(unit_set_m[unit_index_m]);
            std::size_t base_unit_index(current_base_unit_index());
            double      base_value(to_base_value(real, unit));
            double      pinned_base_value = 
                to_pinned_base_value(real, unit,  base_unit_set_m[base_unit_index].min_m,
                                     base_unit_set_m[base_unit_index].max_m);

            // squelch the value if it pins
            squelch = base_value != pinned_base_value;
        }

        // REVISIT (fbrereto) : Bring this back up.
#if 0
        if (select_with_text && using_popup())
            popup_m.select_with_text(candidate);
#endif
    }
}

/*************************************************************************************************/

void edit_number_t::refresh_enabled_state(edit_number_t::controller_t* src)
{
    // this is called by a subcontroller when its enable state changes.
    // we are only concerned with changing our actual enable state when
    // the controller that changed is also the current controller.

    if (&controller_set_m[current_base_unit_index()] != src)
        return;

    edit_text_m.enable(src->enabled_m);
}

/*************************************************************************************************/

void edit_number_t::refresh_view(edit_number_t::view_t* src, const model_type& new_value,
                                 bool force)
{
    // this is called by a subview when its value changes.
    // we are only concerned with changing our actual value when
    // the view that changed is also the current view.

    std::size_t base_unit_index(current_base_unit_index());
    std::string num_str(number_formatter_m.format(new_value));

    if (&view_set_m[base_unit_index] != src)
    {
        // if this isn't the currently visible view,
        // we still need to update its debounce value.

        debounce_type* value(0);

        for (std::size_t i(0); i < base_unit_set_m.size(); ++i)
        {
            if (&view_set_m[i] == src)
            {
                value = &debounce_set_m[i];
                break;
            }
        }

        if (value)
            *value = new_value;

        return;
    }

    debounce_type& debounce(debounce_set_m[base_unit_index]);

    if (debounce == new_value && !force)
        return;

    unit_t& unit(unit_set_m[unit_index_m]);

    debounce = new_value;

    // now we scale the current value with
    // the current unit scale for display

    double scaled_value(to_scaled_value(new_value, unit));

    number_formatter_m.set_format(unit.format_m);

    edit_text_m.display(number_formatter_m.format(scaled_value));
}

/*************************************************************************************************/

std::size_t edit_number_t::current_base_unit_index()
{
    name_t base_unit_name(unit_set_m[unit_index_m].base_unit_m);
    std::size_t   count(base_unit_set_m.size());

    for (std::size_t i(0); i < count; ++i)
        if (base_unit_set_m[i].name_m == base_unit_name)
            return i;

    throw std::runtime_error("base unit not found");
}

/*************************************************************************************************/

void edit_number_t::initialize()
{
    // The first thing we need to do is construct the controller and view vectors

    base_unit_set_m = base_unit_set(unit_set_m.begin(), unit_set_m.end());

    std::size_t count(base_unit_set_m.size());

    // we initialize the debounce_set to std::numeric_limits<debounce_type>::min() instead
    // of 0 because if the value is initialized to 0 it will be debounced, which is wrong.

    debounce_set_m = debounce_set_t(count,
                                    std::numeric_limits<debounce_type>::is_bounded ?
                                        (std::numeric_limits<debounce_type>::min)() :
                                        debounce_type());

    for (std::size_t i(0); i < count; ++i)
    {
        view_set_m.push_back(view_type(this, base_unit_set_m[i].name_m));
        controller_set_m.push_back(controller_type(this, base_unit_set_m[i].name_m));
    }

    // Now we set up the bindings to the subwidgets

    edit_text_m.signal_pre_edit(boost::bind(&edit_number_t::field_text_filter, 
                                            boost::ref(*this), _1, _2));
    edit_text_m.monitor(boost::bind(&edit_number_t::monitor_text, boost::ref(*this), _1, true));

    if (!using_popup())
    {
        platform_m.initialize();

        return;
    }

    // for each item in the unit_set we have to construct an associated popup item to show

    popup_t::menu_item_set_t set;

    set.reserve(unit_set_m.size());

    for (unit_set_t::iterator iter(unit_set_m.begin()), last(unit_set_m.end()); iter != last;
         ++iter)
    {
        popup_t::menu_item_t cur;

        cur.first = iter->name_m;
        cur.second = any_regular_t(iter->name_m);

        set.push_back(cur);
    }

    popup_m.reset_menu_item_set(set);

    platform_m.initialize();
}

/*************************************************************************************************/

void edit_number_t::controller_t::monitor(const setter_proc_t& value)
{
    setter_m = value;
}

/*************************************************************************************************/

void edit_number_t::monitor_locale(const dictionary_t&)
{
    // there's a new locale, so we just want to update the text that's already there
    // with the new locale's formatting

    std::size_t base_unit_index(current_base_unit_index());

    refresh_view(&view_set_m[base_unit_index], debounce_set_m[base_unit_index], true);
}

/*************************************************************************************************/

void edit_number_t::controller_t::enable(bool make_enabled)
{
    assert(control_m);

    if (enabled_m == make_enabled)
        return;

    enabled_m = make_enabled;

    control_m->refresh_enabled_state(this);
}

/*************************************************************************************************/

void edit_number_t::view_t::display(const model_type& value)
{
    assert(control_m);

    control_m->refresh_view(this, value);
}       

/*************************************************************************************************/

void edit_number_t::increment(bool up)
{
    increment_n(up ? 1 : -1);
}

/*************************************************************************************************/

void edit_number_t::increment_n(long n)
{
    std::size_t base_unit_index(current_base_unit_index());
    unit_t&     unit(unit_set_m[unit_index_m]);
    model_type  raw_value(debounce_set_m[base_unit_index]);
    model_type  scaled_value(to_scaled_value(raw_value, unit));
    double      increment(unit.increment_m * n);
    model_type  new_value(scaled_value + increment);

    monitor_text(number_formatter_m.format(new_value), false);
}

/*************************************************************************************************/

} // namespace adobe 

/*************************************************************************************************/
