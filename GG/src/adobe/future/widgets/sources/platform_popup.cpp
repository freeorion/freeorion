/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#include <GG/adobe/future/widgets/headers/display.hpp>
#include <GG/adobe/future/widgets/headers/widget_utils.hpp>
#include <GG/adobe/future/widgets/headers/platform_metrics.hpp>
#include <GG/adobe/future/widgets/headers/platform_popup.hpp>
#include <GG/adobe/future/widgets/headers/platform_widget_utils.hpp>
#include <GG/adobe/placeable_concept.hpp>

#include <GG/DropDownList.h>
#include <GG/GUI.h>
#include <GG/StyleFactory.h>
#include <GG/TextControl.h>


/****************************************************************************************************/

namespace {

/****************************************************************************************************/

GG::Y RowHeight()
{
    static GG::Y retval = GG::Y0;
    if (!retval) {
        const GG::Y DEFAULT_MARGIN(8); // DEFAULT_ROW_HEIGHT from ListBox.cpp, minus the default font's lineskip of 14
        retval = adobe::implementation::DefaultFont()->Lineskip() + DEFAULT_MARGIN;
    }
    return retval;
}

/****************************************************************************************************/

enum metrics {
    gap = 4 // Measured as space from popup to label.
};

/****************************************************************************************************/

void clear_menu_items(adobe::popup_t& control)
{
    assert(control.control_m);
    control.menu_items_m.clear();
    control.control_m->Clear();
}

/****************************************************************************************************/

void sel_changed_slot(adobe::popup_t& popup, GG::DropDownList::iterator it)
{
    assert(popup.control_m);

    if (!popup.value_proc_m.empty() || !popup.extended_value_proc_m.empty())
    {
        std::size_t new_index(popup.control_m->CurrentItemIndex());

        if (popup.custom_m)
            --new_index;

        if (popup.value_proc_m)
            popup.value_proc_m(popup.menu_items_m.at(new_index).second);

        if (popup.extended_value_proc_m)
            popup.extended_value_proc_m(popup.menu_items_m.at(new_index).second, adobe::modifier_state());
    }
}

/****************************************************************************************************/

void set_menu_item_set(adobe::popup_t& p, const adobe::popup_t::menu_item_t* first, const adobe::popup_t::menu_item_t* last)
{
    p.custom_m = false;

    for (; first != last; ++first)
    {
        // MM: Revisit. Is there a way to have disabled separators in combo boxes?
        // Since I don't know a way I intercept -'s here. (Dashes inidcate separators
        // on the macintosh and also in eve at the moment).

        if (first->first != "-" && first->first != "__separator") 
            p.menu_items_m.push_back(*first);
    }
}

/****************************************************************************************************/

void message_menu_item_set(adobe::popup_t& popup)
{
    assert(popup.control_m);

    for (adobe::popup_t::menu_item_set_t::const_iterator
             first = popup.menu_items_m.begin(), last = popup.menu_items_m.end();
         first != last;
         ++first) {
        GG::ListBox::Row* row = new GG::ListBox::Row(GG::X1, RowHeight(), "");
        row->push_back(adobe::implementation::Factory().NewTextControl(GG::X0, GG::Y0, first->first,
                                                                       adobe::implementation::DefaultFont(),
                                                                       GG::CLR_BLACK, GG::FORMAT_LEFT));
        popup.control_m->Insert(row);
    }

    popup.enable(!popup.menu_items_m.empty());

    if (popup.menu_items_m.empty())
        return;

    popup.display(popup.last_m);
}

/****************************************************************************************************/

} // namespace

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

popup_t::popup_t(const std::string& name,
                 const std::string& alt_text,
                 const std::string& custom_item_name,
                 const menu_item_t* first,
                 const menu_item_t* last,
                 theme_t            theme) :
    control_m(0),
    theme_m(theme),
    name_m(name, alt_text, 0, GG::FORMAT_NONE, theme),
    alt_text_m(alt_text),
    using_label_m(!name.empty()),
    custom_m(false),
    custom_item_name_m(custom_item_name)
{
    ::set_menu_item_set(*this, first, last);
}

/****************************************************************************************************/

void popup_t::measure(extents_t& result)
{
    assert(control_m);

    //
    // The popup_t has multiple text items. We need to find the one with
    // the widest extents (when drawn). Then we can make sure that we get
    // enough space to draw our largest text item.
    //
    const boost::shared_ptr<GG::Font>& font = implementation::DefaultFont();

    menu_item_set_t::iterator first(menu_items_m.begin());
    menu_item_set_t::iterator last(menu_items_m.end());
    GG::Pt largest_extent;
    bool have_extents = false;
    for (; first != last; ++first)
    {
        GG::Pt extent = font->TextExtent(first->first);
        if (largest_extent.x < extent.x)
            largest_extent = extent;
        have_extents = true;
    }

    GG::Pt non_client_size = implementation::NonClientSize(*control_m);

    result.width() = Value(largest_extent.x + non_client_size.x);
    result.height() = original_height_m;
    GG::Y baseline =
        (static_cast<int>(result.height()) - implementation::CharHeight()) / 2 +
        implementation::DefaultFont()->Ascent();
    result.vertical().guide_set_m.push_back(Value(baseline));

    //
    // If we have a label (always on our left side?) then we
    // need to add the size of the label to our result. We try
    // to align the label with the popup by baseline. Which is
    // kind of what Eve does, so it's bad that we do this
    // ourselves, really...
    //
    if (!using_label_m)
        return;
    //
    // We store the height of the label, from this we can
    // figure out how much to offset it when positioning
    // the widgets in set_bounds.
    //
    extents_t label_bounds(measure_text(name_m.name_m, font));
    label_bounds.vertical().guide_set_m.push_back(Value(font->Ascent()));
    //
    // Now we can align the label within the vertical
    // slice of the result. This doesn't do anything if
    // the label is shorter than the popup.
    //
    align_slices(result.vertical(), label_bounds.vertical());
    //
    // Add the width of the label (plus a gap) to the
    // resulting width.
    //
    result.width() += gap + label_bounds.width();

    //
    // Don't let the width of the popup go too crazy now...
    //
    result.width() = std::min(result.width(), 300L); // REVISIT (fbrereto) : fixed width

    //
    // Add a point-of-interest where the label ends.
    // We put the label to the left of the popup.
    //
    result.horizontal().guide_set_m.push_back(label_bounds.width());
}

/****************************************************************************************************/

void popup_t::place(const place_data_t& place_data)
{
    using adobe::place;

    assert(control_m);

    place_data_t local_place_data(place_data);

    if (using_label_m)
    {
        //
        // The vertical offset of the label is the geometry's
        // baseline - the label's baseline.
        //
        assert(place_data.vertical().guide_set_m.empty() == false);

        place_data_t label_place_data;
        label_place_data.horizontal().position_m = left(place_data);
        label_place_data.vertical().position_m = top(place_data);

        //
        // The width of the label is the first horizontal
        // point of interest.
        //
        assert(place_data.horizontal().guide_set_m.empty() == false);
        width(label_place_data) = place_data.horizontal().guide_set_m[0];
        height(label_place_data) = height(place_data);

        //
        // Set the label dimensions.
        //
        place(name_m, label_place_data);

        //
        // Now we need to adjust the position of the popup
        // widget.
        //
        long width = gap + adobe::width(label_place_data);
        local_place_data.horizontal().position_m += width;
        adobe::width(local_place_data) -= width;
    }

    implementation::set_control_bounds(control_m, local_place_data);
}

/****************************************************************************************************/

void popup_t::enable(bool make_enabled)
{
    assert(control_m);
    control_m->Disable(!make_enabled);
}

/****************************************************************************************************/

void popup_t::reset_menu_item_set(const menu_item_t* first, const menu_item_t* last)
{
    assert(control_m);
    clear_menu_items(*this);
    ::set_menu_item_set(*this, first, last);
    ::message_menu_item_set(*this);
}

/****************************************************************************************************/

void popup_t::display(const model_type& value)
{
    assert(control_m);

    last_m = value;

    menu_item_set_t::iterator  first(menu_items_m.begin());
    menu_item_set_t::iterator  last(menu_items_m.end());

    for (; first != last; ++first) {
        if ((*first).second == value) {
            if (custom_m) {
                custom_m = false;
                control_m->Erase(control_m->begin());
            }
            std::ptrdiff_t index(first - menu_items_m.begin());
            control_m->Select(index);
            return;
        }
    }

    display_custom();
}

/****************************************************************************************************/

void popup_t::display_custom()
{
    if (custom_m)
        return;

    custom_m = true;
    GG::ListBox::Row* row = new GG::ListBox::Row(GG::X1, RowHeight(), "");
    row->push_back(adobe::implementation::Factory().NewTextControl(GG::X0, GG::Y0, custom_item_name_m,
                                                                   adobe::implementation::DefaultFont(),
                                                                   GG::CLR_BLACK, GG::FORMAT_LEFT));
    control_m->Insert(row, control_m->begin());
    control_m->Select(0);
}

/****************************************************************************************************/

void popup_t::select_with_text(const std::string& text)
{
    assert(control_m);

    std::size_t old_index(control_m->CurrentItemIndex());

    std::size_t new_index = std::numeric_limits<std::size_t>::max();
    for (std::size_t i = 0; i < menu_items_m.size(); ++i) {
        if (menu_items_m[i].first == text) {
            new_index = i;
            break;
        }
    }

    if (new_index < menu_items_m.size())
        control_m->Select(new_index);

    if (value_proc_m.empty())
        return;

    if (new_index != old_index)
        value_proc_m(menu_items_m.at(new_index).second);
}

/****************************************************************************************************/

void popup_t::monitor(const setter_type& proc)
{
    assert(control_m);
    value_proc_m = proc;
}

/****************************************************************************************************/

void popup_t::monitor_extended(const extended_setter_type& proc)
{
    assert(control_m);
    extended_value_proc_m = proc;
}

/****************************************************************************************************/

// REVISIT: MM--we need to replace the display_t mechanism with concepts/any_*/container idiom for event and drawing system.

template <> platform_display_type insert<popup_t>(display_t&             display,
                                                  platform_display_type& parent,
                                                  popup_t&               element)
{
    if (element.using_label_m)
        insert(display, parent, element.name_m);

    assert(!element.control_m);

    int lines = std::min(element.menu_items_m.size(), std::size_t(20u));
    element.control_m =
        implementation::Factory().NewDropDownList(GG::X0, GG::Y0, GG::X1, implementation::StandardHeight(),
                                                  RowHeight() * lines + static_cast<int>(2 * GG::ListBox::BORDER_THICK),
                                                  GG::CLR_GRAY);

    element.original_height_m = Value(element.control_m->Height());

    GG::Connect(element.control_m->SelChangedSignal,
                boost::bind(&sel_changed_slot, boost::ref(element), _1));

    if (!element.alt_text_m.empty())
        adobe::implementation::set_control_alt_text(element.control_m, element.alt_text_m);

    ::message_menu_item_set(element);

    return display.insert(parent, element.control_m);
}

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/
