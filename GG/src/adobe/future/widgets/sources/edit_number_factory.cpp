/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#define ADOBE_DLL_SAFE 0

// .hpp needs to come before widget_factory to hook the overrides
#include <GG/adobe/future/widgets/headers/edit_number.hpp>
#include <GG/adobe/future/widgets/headers/display.hpp>
#include <GG/adobe/future/widgets/headers/number_unit.hpp>

#include <GG/adobe/future/widgets/headers/edit_number_factory.hpp>
#include <GG/adobe/future/widgets/headers/widget_factory.hpp>
#include <GG/adobe/future/widgets/headers/widget_factory_registry.hpp>
#include <GG/adobe/layout_attributes.hpp>

#include <limits>

/****************************************************************************************************/

namespace {

/****************************************************************************************************/

std::vector<adobe::unit_t> extract_unit_set(const adobe::dictionary_t& parameters)
{
    std::vector<adobe::unit_t> result;
    adobe::unit_t              default_unit(adobe::to_unit(parameters));

    if (parameters.count(adobe::key_units) == 0)
    {
        result.push_back(default_unit);
    }
    else
    {
        adobe::array_t unit_array(get_value(parameters, adobe::key_units).cast<adobe::array_t>());

        for (adobe::array_t::iterator iter(unit_array.begin()), last(unit_array.end());
             iter != last; ++iter)
            result.push_back(adobe::to_unit(iter->cast<adobe::dictionary_t>(), default_unit));
    }

    return result;
}

/**************************************************************************     **************************/

void attach_edit_num_view_and_controller(adobe::edit_number_t& control,
                                const adobe::dictionary_t&     parameters,
                                const adobe::factory_token_t&  token)
{
    adobe::basic_sheet_t& layout_sheet(token.client_holder_m.layout_sheet_m);
    adobe::assemblage_t&  assemblage(token.client_holder_m.assemblage_m);

    for (adobe::edit_number_t::view_iterator iter(control.view_begin()), last(control.view_end());
         iter != last; ++iter)
    {
        adobe::name_t cell(iter->cell());

        if (cell == adobe::name_t())
            continue;

        adobe::attach_view_direct(*iter, parameters, token, cell);
    }

    for (adobe::edit_number_t::controller_iterator iter(control.controller_begin()), last(control.controller_end());
         iter != last; ++iter)
    {
        adobe::name_t cell(iter->cell());

        if (cell == adobe::name_t())
            continue;

        if (layout_sheet.count_interface(cell) != 0)
            adobe::couple_controller_to_cell(*iter, cell, layout_sheet, token, parameters);
        else
            adobe::couple_controller_to_cell(*iter, cell, token.sheet_m, token, parameters);
    }

    if (!control.using_popup())
        return;

    // lastly, we bind the unit popup (if there is one) and the monitor_popup call in edit_number
    // to a cell in the layout sheet, that is either specified by the client or is uniquely
    // generated.
    {
        adobe::name_t              cell(adobe::state_cell_unique_name());
        std::vector<adobe::unit_t> unit_set(extract_unit_set(parameters));

        get_value(parameters, adobe::key_bind_units, cell);

        // add the cell to the layout sheet if its not there yet
        if (layout_sheet.count_interface(cell) == 0)
            layout_sheet.add_interface(cell, adobe::any_regular_t(unit_set.front().name_m));

        // attach the popup to the cell as a normal widget
        adobe::attach_view(assemblage, cell, control.popup(), layout_sheet);
        adobe::attach_monitor(control.popup(), cell, layout_sheet, token, parameters);

        // When the popup value changes, we want wind of it, too, to update the number
        adobe::edit_number_unit_subwidget_t* tmp = new adobe::edit_number_unit_subwidget_t(control);

        assemblage_cleanup_ptr(assemblage, tmp);

        adobe::attach_view(assemblage, cell, *tmp, layout_sheet);
    }

    if (parameters.count(adobe::key_bind_group))
    {
        adobe::array_t bind_group_set;

        get_value(parameters, adobe::key_bind_group, bind_group_set);

        // set up the splitter controller
        typedef adobe::splitter_controller_adaptor<adobe::any_regular_t> splitter_type;

        splitter_type* splitter =
            new splitter_type(token.client_holder_m.layout_sheet_m,
                              token.client_holder_m.root_behavior_m,
                              bind_group_set,
                              adobe::modifiers_any_shift_s);

        control.popup().monitor_extended(boost::ref(*splitter));

        assemblage_cleanup_ptr(assemblage, splitter);
    }
}

/****************************************************************************************************/

} //namespace

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

void create_widget(const dictionary_t& parameters,
                   size_enum_t         size,
                   edit_number_t*&     edit_number)
{
    edit_text_ctor_block_t block;

    block.theme_m = implementation::size_to_theme(size);

    get_value(parameters, key_name, block.name_m);
    get_value(parameters, key_alt_text, block.alt_text_m);
    get_value(parameters, key_digits, block.min_characters_m);
    get_value(parameters, key_max_digits, block.max_characters_m);

    std::vector<unit_t> unit_set(extract_unit_set(parameters));

    edit_number = new edit_number_t(block, unit_set.begin(), unit_set.end());
}

/****************************************************************************************************/


template <>
platform_display_type insert<edit_number_t>(display_t&             display,
                                            platform_display_type& parent,
                                            edit_number_t&  element)
{
    platform_display_type pos(insert(display, parent, element.edit_text()));
        
        if (element.using_popup())
        insert(display, parent, element.popup());

    element.initialize();

    element.locale_change_connection_m =
        monitor_locale(boost::bind(&edit_number_t::monitor_locale, boost::ref(element), _1));

    return pos;
}

/****************************************************************************************************/

widget_node_t make_edit_number(const dictionary_t&     parameters, 
                               const widget_node_t&    parent, 
                               const factory_token_t&  token,
                               const widget_factory_t& factory)
{
    bool is_container(factory.is_container(static_name_t("edit_number")));
    const layout_attributes_t& layout_attributes(
        factory.layout_attributes(static_name_t("edit_number")));

    size_enum_t   size(parameters.count(key_size) ?
                       implementation::enumerate_size(get_value(parameters, key_size).cast<name_t>()) :
                       parent.size_m);

    edit_number_t* widget(NULL);
    create_widget(parameters, size, widget);
    assemblage_cleanup_ptr(token.client_holder_m.assemblage_m, widget);

    //
    // Call display_insertion to embed the new widget within the view heirarchy
    //
    platform_display_type display_token(insert(get_main_display(), parent.display_token_m, *widget));

    //
    // As per SF.net bug 1428833, we want to attach the poly_placeable_t
    // to Eve before we attach the controller and view to the model
    //

    eve_t::iterator eve_token;
    eve_token = attach_placeable<poly_placeable_t>(parent.eve_token_m, *widget, parameters, token, is_container, layout_attributes);

    attach_edit_num_view_and_controller(*widget, parameters, token);

    keyboard_t::iterator keyboard_token = keyboard_t::get().insert(parent.keyboard_token_m, poly_key_handler_t(boost::ref(*widget)));

    //
    // Return the widget_node_t that comprises the tokens created for this widget by the various components
    //

    return widget_node_t(size, eve_token, display_token, keyboard_token);
}


/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/
