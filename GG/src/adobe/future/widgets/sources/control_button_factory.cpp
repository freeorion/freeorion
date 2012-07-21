/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#define ADOBE_DLL_SAFE 0

// .hpp needs to come before widget_factory to hook the overrides
#include <GG/adobe/future/widgets/headers/control_button.hpp>

#include <GG/adobe/adam_parser.hpp>
#include <GG/adobe/future/widgets/headers/control_button_factory.hpp>
#include <GG/adobe/future/widgets/headers/widget_factory.hpp>
#include <GG/adobe/future/widgets/headers/widget_factory_registry.hpp>
#include <GG/adobe/layout_attributes.hpp>

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/


template <>
platform_display_type insert<control_button_t>(display_t&             display,
                                               platform_display_type& parent,
                                               control_button_t&      element)
{
    assert(element.button_m.get());

    return insert(display, parent, *(element.button_m));
}

/*************************************************************************************************/

template <typename Sheet>
void attach_view(assemblage_t&,
                 name_t,
                 control_button_t&,
                 Sheet&)
{ }

/*************************************************************************************************/

template <>
inline widget_node_t
create_and_hookup_widget<control_button_t, poly_placeable_t>(const dictionary_t&        parameters,
                                                             const widget_node_t&       parent,
                                                             const factory_token_t&     token,
                                                             bool                       is_container,
                                                             const layout_attributes_t& layout_attributes)
{
    size_enum_t   size(parameters.count(key_size) ?
                       implementation::enumerate_size(get_value(parameters, key_size).cast<name_t>()) :
                       parent.size_m);
    std::string   name;
    std::string   alt_text;
    std::string   expression_string;

    get_value(parameters, key_name, name);
    get_value(parameters, key_alt_text, alt_text);
    get_value(parameters, static_name_t("expression"), expression_string);

    control_button_t* widget(NULL);

    widget = new control_button_t(name,
                                  alt_text,
                                  boost::bind(&sheet_t::inspect, boost::ref(token.sheet_m), _1),
                                  parse_adam_expression(expression_string),
                                  implementation::size_to_theme(size));

    assemblage_cleanup_ptr(token.client_holder_m.assemblage_m, widget);

    //
    // Call display_insertion to embed the new widget within the view heirarchy
    //
    platform_display_type display_token(insert(get_main_display(), parent.display_token_m, *widget));

    eve_t::iterator eve_token;

    eve_token = attach_placeable<poly_placeable_t>(parent.eve_token_m,
                                                   *widget,
                                                   parameters,
                                                   token,
                                                   is_container,
                                                   layout_attributes);

    attach_view_and_controller(*widget, parameters, token);

    //
    // Return the widget_node_t that comprises the tokens created for this widget by the various components
    //
    return widget_node_t(size, eve_token, display_token, parent.keyboard_token_m);
}

/****************************************************************************************************/

widget_node_t make_control_button(const dictionary_t&     parameters, 
                                  const widget_node_t&    parent, 
                                  const factory_token_t&  token,
                                  const widget_factory_t& factory)
{
    return create_and_hookup_widget<control_button_t, poly_placeable_t>(parameters, parent, token, 
               factory.is_container(static_name_t("control_button")), 
               factory.layout_attributes(static_name_t("control_button")));
}


/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/
