/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

// radio_button.hpp needs to come before widget_factory to hook the overrides
#include <GG/adobe/future/widgets/headers/platform_radio_button.hpp>

#include <GG/adobe/future/widgets/headers/radio_button_factory.hpp>
#include <GG/adobe/future/widgets/headers/widget_factory.hpp>
#include <GG/adobe/future/widgets/headers/widget_factory_registry.hpp>

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

void create_widget(const dictionary_t& parameters,
                   size_enum_t         size,
                   radio_button_t*&     widget)
{
    std::string   name;
    std::string   alt_text;
    any_regular_t set_value;

    get_value(parameters, key_name, name);
    get_value(parameters, key_alt_text, alt_text);
    get_value(parameters, key_value, set_value);

    widget = new radio_button_t(name, alt_text, set_value, implementation::size_to_theme(size));
}
    
/****************************************************************************************************/

widget_node_t make_radio_button(const dictionary_t&     parameters,
                                const widget_node_t&    parent,
                                const factory_token_t&  token,
                                const widget_factory_t& factory)
{ 
    return create_and_hookup_widget<radio_button_t, poly_placeable_t>(parameters, parent, token, 
        factory.is_container(static_name_t("radio_button")), 
        factory.layout_attributes(static_name_t("radio_button"))); 
}

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/
