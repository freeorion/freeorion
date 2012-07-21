/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#define ADOBE_DLL_SAFE 0

#include <GG/adobe/future/widgets/headers/platform_image.hpp>

#include <GG/adobe/future/resources.hpp>
#include <GG/adobe/future/widgets/headers/factory.hpp>
#include <GG/adobe/future/widgets/headers/image_factory.hpp>
#include <GG/adobe/future/widgets/headers/virtual_machine_extension.hpp>
#include <GG/adobe/future/widgets/headers/widget_factory_registry.hpp>
#include <GG/adobe/future/widgets/headers/widget_factory.hpp>
#include <GG/adobe/future/widgets/headers/widget_utils.hpp>

#include <GG/GUI.h>


/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/

void create_widget(const dictionary_t& parameters,
                   size_enum_t         /*size*/,
                   image_t*&           widget)
{
    std::string              image_filename;
    image_t::view_model_type actual_image;

    get_value(parameters, key_image, image_filename);

    try
    { actual_image = GG::GUI::GetGUI()->GetTexture(image_filename); }
    catch (...) {}

    widget = new image_t(actual_image);
}

/*************************************************************************************************/

void subscribe_view_to_model(image_t&                control,
                             name_t                  cell, 
                             basic_sheet_t*          layout_sheet,
                             sheet_t*                model_sheet,
                             assemblage_t&           assemblage,
                             visible_change_queue_t& visible_queue)
{
    typedef force_relayout_view_adaptor<image_t> adaptor_type;

    adaptor_type* view_adaptor(new adaptor_type(control, visible_queue));

    assemblage_cleanup_ptr(assemblage, view_adaptor);

    if (layout_sheet)
    {
        attach_view(assemblage, cell, *view_adaptor, *layout_sheet);
        return;
    }

    attach_view(assemblage, cell, *view_adaptor, *model_sheet);
}

/****************************************************************************************************/

namespace implementation {

/*************************************************************************************************/

widget_node_t make_image_hack(const dictionary_t&     parameters, 
                              const widget_node_t&    parent, 
                              const factory_token_t&  token,
                              const widget_factory_t& factory)
    { return create_and_hookup_widget<image_t, poly_placeable_twopass_t>(
        parameters, parent, token, 
        factory.is_container(static_name_t("image")), 
        factory.layout_attributes(static_name_t("image"))); }

/*************************************************************************************************/

} // namespace implementation

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/
