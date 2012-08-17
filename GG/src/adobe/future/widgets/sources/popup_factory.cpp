/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

// popup.hpp needs to come before widget_factory to hook the overrides
#include <GG/adobe/future/widgets/headers/platform_popup.hpp>

#include <GG/adobe/future/widgets/headers/popup_common.hpp>
#include <GG/adobe/future/widgets/headers/popup_factory.hpp>
#include <GG/adobe/future/widgets/headers/widget_factory.hpp>
#include <GG/adobe/future/widgets/headers/widget_factory_registry.hpp>

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

void create_widget(const dictionary_t& parameters,
                   size_enum_t         size,
                   popup_t*&           widget)
{
    std::string              name;
    std::string              alt_text;
    std::string              custom_item_name("Custom");
    array_t           items;
    theme_t           theme(implementation::size_to_theme(size));
    popup_t::menu_item_set_t item_set;

    get_value(parameters, key_name, name);
    get_value(parameters, key_alt_text, alt_text);
    get_value(parameters, key_items, items);
    get_value(parameters, key_custom_item_name, custom_item_name);

    for (array_t::iterator first(items.begin()), last(items.end()); first != last; ++first)
    {
        dictionary_t cur_item(first->cast<dictionary_t>());

        item_set.push_back(
            popup_t::menu_item_set_t::value_type(
                get_value(cur_item, key_name).cast<std::string>(), get_value(cur_item, key_value)));
    }

    // REVISIT (fbrereto) : Should be called 'first' but for the fact that MSVC 7.1 complains.
    popup_t::menu_item_set_t::value_type* first_value(item_set.empty() ? 0 : &item_set[0]);

    widget = new popup_t(name, alt_text, custom_item_name,
                       first_value, first_value + item_set.size(),
                       theme);
}

/*************************************************************************************************/

template <>
void attach_view_and_controller(popup_t&               control,
                                const dictionary_t&    parameters,
                                const factory_token_t& token,
                                adobe::name_t,
                                adobe::name_t,
                                adobe::name_t)
{
    basic_sheet_t& layout_sheet(token.client_holder_m.layout_sheet_m);
    assemblage_t&  assemblage(token.client_holder_m.assemblage_m);

    if (parameters.count(key_bind) != 0)
    {
        name_t cell(get_value(parameters, key_bind).cast<name_t>());

        attach_view_and_controller_direct(control, parameters, token, cell);
    }

    if (parameters.count(key_items) && 
            get_value(parameters, key_items).type_info() == type_info<name_t>())
    {
        // dynamically bind to the cell instead of getting a static list of popup items

        name_t cell(get_value(parameters, key_items).cast<name_t>());

        if (layout_sheet.count_interface(cell))
            attach_popup_menu_item_set(control, cell, layout_sheet, assemblage, token.client_holder_m);
        else
            attach_popup_menu_item_set(control, cell, token.sheet_m, assemblage, token.client_holder_m);
    }
}

/****************************************************************************************************/

widget_node_t make_popup(const dictionary_t&     parameters,
                         const widget_node_t&    parent,
                         const factory_token_t&  token,
                         const widget_factory_t& factory)
{ 
    return create_and_hookup_widget<popup_t, poly_placeable_t>(parameters, parent, token, 
        factory.is_container(static_name_t("popup")), 
        factory.layout_attributes(static_name_t("popup"))); 
}

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/
