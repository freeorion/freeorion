/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

// optional_panel.hpp needs to come before widget_factory to hook the overrides
#include <GG/adobe/future/widgets/headers/platform_optional_panel.hpp>

#include <GG/adobe/future/widgets/headers/optional_panel_factory.hpp>
#include <GG/adobe/future/widgets/headers/widget_factory.hpp>
#include <GG/adobe/future/widgets/headers/widget_factory_registry.hpp>
#include <GG/adobe/dictionary.hpp>

/*************************************************************************************************/

namespace {

/*************************************************************************************************/

void optional_display_show(adobe::visible_change_queue_t&  visible_change_queue,
                           adobe::eve_t&                   eve_layout,
                           adobe::eve_t::iterator          eve_token,
                           const boost::function<void ()>& theproc)
{
    visible_change_queue.show_queue_m->insert(theproc);

    eve_layout.set_visible(eve_token, true);
}

/*************************************************************************************************/

void optional_display_hide(adobe::visible_change_queue_t&  visible_change_queue,
                           adobe::eve_t&                   eve_layout,
                           adobe::eve_t::iterator          eve_token,
                           const boost::function<void ()>& theproc)
{
    visible_change_queue.hide_queue_m->insert(theproc);

    eve_layout.set_visible(eve_token, false);
}

/*************************************************************************************************/

} //namespace 

/*************************************************************************************************/

namespace adobe { 

/*************************************************************************************************/

void create_widget(const dictionary_t& parameters,
                   size_enum_t         size,
                   optional_panel_t*& widget)
{
    any_regular_t value(true);

    get_value(parameters, key_value, value);

    widget = new optional_panel_t(value, implementation::size_to_theme(size));
}

/*************************************************************************************************/

template <typename Sheet, typename FactoryToken>
inline void couple_controller_to_cell(optional_panel_t&,
                                      name_t,
                                      Sheet&,
                                      const FactoryToken&,
                                      const dictionary_t&)

{
    // no adam interaction
}

/*************************************************************************************************/

widget_node_t make_optional_panel(const dictionary_t&     parameters,
                                  const widget_node_t&    parent,
                                  const factory_token_t&  token,
                                  const widget_factory_t& factory)
{
   size_enum_t   size(parameters.count(key_size) ?
                       implementation::enumerate_size(get_value(parameters, key_size).cast<name_t>()) :
                       parent.size_m);

    optional_panel_t* widget(NULL);
    create_widget(parameters, size, widget);
    token.client_holder_m.assemblage_m.cleanup(boost::bind(delete_ptr<optional_panel_t*>(), widget));
  
    //
    // Call display_insertion to embed the new widget within the view heirarchy
    //
    platform_display_type display_token(insert(get_main_display(), parent.display_token_m, *widget));

    //
    // As per SF.net bug 1428833, we want to attach the poly_placeable_t
    // to Eve before we attach the controller and view to the model
    //

    eve_t::iterator eve_token;
    eve_token = attach_placeable<poly_placeable_t>(parent.eve_token_m, 
        *widget, parameters, token, 
        factory.is_container(static_name_t("optional")), 
        factory.layout_attributes(static_name_t("optional")));

    widget->set_optional_display_procs(boost::bind(&optional_display_show,
                                                   boost::ref(token.client_holder_m.visible_change_queue_m),
                                                   boost::ref(token.client_holder_m.eve_m),
                                                   eve_token,
                                                   _1),
                                       boost::bind(&optional_display_hide,
                                                   boost::ref(token.client_holder_m.visible_change_queue_m),
                                                   boost::ref(token.client_holder_m.eve_m),
                                                   eve_token,
                                                   _1));

    attach_view_and_controller(*widget, parameters, token);

    //
    // Return the widget_node_t that comprises the tokens created for this widget by the various components
    //
    return widget_node_t(size, eve_token, display_token, parent.keyboard_token_m);
}

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/
