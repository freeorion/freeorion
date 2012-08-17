/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

// reveal.hpp needs to come before widget_factory to hook the overrides
#include <GG/adobe/future/widgets/headers/platform_reveal.hpp>

#include <GG/adobe/future/widgets/headers/reveal_factory.hpp>
#include <GG/adobe/future/widgets/headers/widget_factory.hpp>
#include <GG/adobe/future/widgets/headers/widget_factory_registry.hpp>

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

void create_widget(const dictionary_t&  parameters, 
                   size_enum_t          size,
                   reveal_t*&           widget)
{
    std::string             name;
    std::string             alt_text;
    any_regular_t    show_value(true);
    theme_t          theme(implementation::size_to_theme(size));

    get_value(parameters, key_value_on, show_value);
    get_value(parameters, key_alt_text, alt_text);
    get_value(parameters, key_name, name);

    widget = new reveal_t(name, show_value, theme, alt_text);
}

/****************************************************************************************************/

template <typename Sheet>
void couple_controller_to_cell(reveal_t&                 controller,
                               name_t             cell, 
                               Sheet&                    sheet, 
                               const factory_token_t& token, 
                               const dictionary_t&       parameters)
{
    attach_monitor(controller, cell, sheet, token, parameters);
}
    
/****************************************************************************************************/

widget_node_t make_reveal(const dictionary_t&     parameters,
                          const widget_node_t&    parent,
                          const factory_token_t&  token,
                          const widget_factory_t& factory)
{ 
    return create_and_hookup_widget<reveal_t, poly_placeable_t>(parameters, parent, token, 
        factory.is_container(static_name_t("reveal")), 
        factory.layout_attributes(static_name_t("reveal"))); 
}


/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/
