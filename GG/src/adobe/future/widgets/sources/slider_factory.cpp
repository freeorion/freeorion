/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

// slider.hpp needs to come before widget_factory to hook the overrides
#include <GG/adobe/future/widgets/headers/platform_slider.hpp>

#include <GG/adobe/future/widgets/headers/slider_factory.hpp>
#include <GG/adobe/future/widgets/headers/widget_factory.hpp>
#include <GG/adobe/future/widgets/headers/widget_factory_registry.hpp>

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

void create_widget(const dictionary_t& parameters,
                   size_enum_t         size,
                   slider_t*&          widget)
{
    std::string           alt_text;
    name_t         slider_pointing;
    long                  num_ticks(0);
    value_range_format_t  format;
    name_t         orientation(key_horizontal);
    slider_style_t style(slider_points_not_s);

    if (parameters.count(key_format))
        format.set(get_value(parameters, key_format).cast<dictionary_t>());

    get_value(parameters, key_alt_text, alt_text);
    get_value(parameters, key_orientation, orientation);
    get_value(parameters, key_slider_ticks, num_ticks);
    get_value(parameters, key_slider_point, slider_pointing);

    if (slider_pointing == static_name_t("up"))         style = slider_points_up_s;
    else if (slider_pointing == static_name_t("down"))  style = slider_points_down_s;
    else if (slider_pointing == static_name_t("left"))  style = slider_points_left_s;
    else if (slider_pointing == static_name_t("right")) style = slider_points_right_s;

    widget = new slider_t(alt_text,
                                 orientation == key_vertical,
                                 style,
                                 num_ticks,
                                 format,
                                 implementation::size_to_theme(size));
}

/****************************************************************************************************/

template <>
void attach_view_and_controller(slider_t&              control,
                                const dictionary_t&    parameters,
                                const factory_token_t& token,
                                adobe::name_t,
                                adobe::name_t,
                                adobe::name_t)
{
    basic_sheet_t& layout_sheet(token.client_holder_m.layout_sheet_m);

    if (parameters.count(key_bind) != 0)
    {
        name_t cell(get_value(parameters, key_bind).cast<name_t>());
    
        attach_view_direct(control, parameters, token, cell);

        // is the cell in the layout sheet or the Adam sheet?
        if (layout_sheet.count_interface(cell) != 0)
            couple_controller_to_cell(control, cell, layout_sheet, token, parameters);
        else
            couple_controller_to_cell(control, cell, token.sheet_m, token, parameters);
    }
}

/****************************************************************************************************/

widget_node_t make_slider(const dictionary_t&     parameters,
                          const widget_node_t&    parent,
                          const factory_token_t&  token,
                          const widget_factory_t& factory)
{ 
    return create_and_hookup_widget<slider_t, poly_placeable_t>(parameters, parent, token, 
        factory.is_container(static_name_t("slider")), 
        factory.layout_attributes(static_name_t("slider"))); }

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/
