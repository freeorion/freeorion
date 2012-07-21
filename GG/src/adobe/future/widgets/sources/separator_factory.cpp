/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

// separator.hpp needs to come before widget_factory to hook the overrides
#include <GG/adobe/future/widgets/headers/platform_separator.hpp>

#include <GG/adobe/future/widgets/headers/separator_factory.hpp>
#include <GG/adobe/future/widgets/headers/widget_factory.hpp>
#include <GG/adobe/future/widgets/headers/widget_factory_registry.hpp>
#include <GG/adobe/dictionary.hpp>

/*************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

void create_widget(const dictionary_t& parameters,
                   size_enum_t         /*size*/,
                   separator_t*&       widget)
{
    name_t orientation (key_horizontal);
    get_value(parameters, key_orientation, orientation);
    bool is_vertical = (orientation == key_vertical);
    widget = new separator_t(is_vertical, theme_none_s);
}

/****************************************************************************************************/

template <>
void attach_view_and_controller(separator_t&,
                                const dictionary_t&,
                                const factory_token_t&,
                                adobe::name_t,
                                adobe::name_t,
                                adobe::name_t)
{
    // no adam interaction
}

/****************************************************************************************************/

widget_node_t make_separator(const dictionary_t&     parameters,
                             const widget_node_t&    parent,
                             const factory_token_t&  token,
                             const widget_factory_t& factory)
{ 
    return create_and_hookup_widget<separator_t, poly_placeable_t>(parameters, parent, token, 
        factory.is_container(static_name_t("separator")), 
        factory.layout_attributes(static_name_t("separator"))); }

/*************************************************************************************************/

} // namespace adobe
