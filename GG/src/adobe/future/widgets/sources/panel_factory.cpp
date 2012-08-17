/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

// tab_group.hpp needs to come before widget_factory to hook the overrides
#include <GG/adobe/future/widgets/headers/platform_panel.hpp>

#include <GG/adobe/future/widgets/headers/panel_factory.hpp>
#include <GG/adobe/future/widgets/headers/widget_factory.hpp>
#include <GG/adobe/future/widgets/headers/widget_factory_registry.hpp>
#include <GG/adobe/dictionary.hpp>

/*************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

void create_widget(const dictionary_t& parameters,
                   size_enum_t         size,
                   panel_t*&           widget)
{
    //
    // REVISIT (ralpht): It used to be the case that panels would automatically
    //      figure out if they had a tab_control parent, and then bind
    //      to the tab control's visibility cell in the Adam sheet. No
    //      more! Now you have to tell the panel to bind against the
    //      visibility cell and which value it should show for. We might
    //      be able to do something clever with signals, or make the
    //      client_assembler manipulate incoming parameters for children
    //      of tab controls.
    //
    any_regular_t value(true);

    get_value(parameters, key_value, value);

    widget = new panel_t(value, implementation::size_to_theme(size));
}

/*************************************************************************************************/

template <typename Sheet, typename FactoryToken>
inline void couple_controller_to_cell(panel_t&,
                                      name_t,
                                      Sheet&,
                                      const FactoryToken&,
                                      const dictionary_t&)

{
    // no adam interaction
}

/****************************************************************************************************/

widget_node_t make_panel(const dictionary_t&     parameters,
                         const widget_node_t&    parent,
                         const factory_token_t&  token,
                         const widget_factory_t& factory)
{
    return create_and_hookup_widget<panel_t, poly_placeable_t>(parameters, parent, token, 
                                                              factory.is_container(static_name_t("panel")), 
                                                              factory.layout_attributes(static_name_t("panel"))); 
}

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/
