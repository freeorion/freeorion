/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#define ADOBE_DLL_SAFE 0

// toggle.hpp needs to come before widget_factory to hook the overrides
#include <GG/adobe/future/widgets/headers/platform_toggle.hpp>

#include <GG/adobe/functional.hpp>
#include <GG/adobe/future/widgets/headers/toggle_factory.hpp>
#include <GG/adobe/future/widgets/headers/virtual_machine_extension.hpp>
#include <GG/adobe/future/widgets/headers/widget_factory_registry.hpp>
#include <GG/adobe/future/widgets/headers/widget_factory.hpp>

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

void create_widget(const dictionary_t&  parameters, 
                   size_enum_t          size,
                   toggle_t*&           widget)
{
    std::string          alt_text;
    any_regular_t        value_on(true);
    toggle_t::image_type image_on;
    toggle_t::image_type image_off;
    toggle_t::image_type image_disabled;
    theme_t              theme(implementation::size_to_theme(size));

    get_value(parameters, key_alt_text).cast(alt_text);
    get_value(parameters, key_value_on).cast(value_on);
    get_value(parameters, key_image_on).cast(image_on);
    get_value(parameters, key_image_off).cast(image_off);
    get_value(parameters, key_image_disabled).cast(image_disabled);

    widget = new toggle_t(alt_text, value_on, image_on, image_off, image_disabled, theme);
}

/*************************************************************************************************/

void subscribe_view_to_model(toggle_t&               control,
                             name_t                  cell, 
                             basic_sheet_t*          layout_sheet,
                             sheet_t*                model_sheet,
                             assemblage_t&           assemblage,
                             visible_change_queue_t& visible_queue)
{
    typedef force_relayout_view_adaptor<toggle_t> adaptor_type;

    adaptor_type* view_adaptor(new adaptor_type(control, visible_queue));

    assemblage.cleanup(boost::bind(delete_ptr<adaptor_type*>(), view_adaptor));

    if (layout_sheet)
    {
        attach_view(assemblage, cell, *view_adaptor, *layout_sheet);
        return;
    }

    attach_view(assemblage, cell, *view_adaptor, *model_sheet);
}

/****************************************************************************************************/

widget_node_t make_toggle(const dictionary_t&     parameters,
                          const widget_node_t&    parent,
                          const factory_token_t&  token,
                          const widget_factory_t& factory)
{ 
    return create_and_hookup_widget<toggle_t, poly_placeable_t>(parameters, parent, token, 
        factory.is_container(static_name_t("toggle")), 
        factory.layout_attributes(static_name_t("toggle"))); 
}


/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/
