/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#define ADOBE_DLL_SAFE 0

// button.hpp needs to come before widget_factory to hook the overrides
#include <GG/adobe/future/widgets/headers/platform_button.hpp>
#include <GG/adobe/functional.hpp>
#include <GG/adobe/future/widgets/headers/button_helper.hpp>
#include <GG/adobe/future/widgets/headers/button_factory.hpp>
#include <GG/adobe/future/widgets/headers/widget_factory.hpp>
#include <GG/adobe/future/widgets/headers/widget_factory_registry.hpp>

/****************************************************************************************************/

namespace {

/****************************************************************************************************/

struct button_item_t
{
    button_item_t() : modifier_set_m(adobe::modifiers_none_s)
    { }

    button_item_t(const button_item_t& default_item, const adobe::dictionary_t& parameters) :
        name_m(default_item.name_m),
        alt_text_m(default_item.alt_text_m),
        bind_m(default_item.bind_m),
        bind_output_m(default_item.bind_output_m),
        action_m(default_item.action_m),
        value_m(default_item.value_m),
        contributing_m(default_item.contributing_m),
        modifier_set_m(default_item.modifier_set_m)
    {
        get_value(parameters, adobe::key_name, name_m);
        get_value(parameters, adobe::key_alt_text, alt_text_m);
        get_value(parameters, adobe::key_bind, bind_m);
        get_value(parameters, adobe::key_bind_output, bind_output_m);
        get_value(parameters, adobe::key_action, action_m);
        get_value(parameters, adobe::key_value, value_m);
        
        // modifers can be a name or array

        /*
            REVISIT (sparent) : Old way was with a single modifers key word with multiple
            modifers encoded in a single name - new way with a name or array under the
            new keyword modifier_set.
        */
        adobe::dictionary_t::const_iterator iter(parameters.find(adobe::key_modifiers));

        if (iter == parameters.end())
            iter = parameters.find(adobe::key_modifier_set);
        
        if (iter != parameters.end())
            modifier_set_m |= adobe::value_to_modifier(iter->second);
    }

    std::string          name_m;
    std::string          alt_text_m;
    adobe::name_t        bind_m;
    adobe::name_t        bind_output_m;
    adobe::name_t        action_m;
    adobe::any_regular_t value_m;
    adobe::dictionary_t  contributing_m;
    adobe::modifiers_t   modifier_set_m;  
};

/*************************************************************************************************/

void proxy_button_hit(  adobe::button_notifier_t    notifier,
                        adobe::sheet_t&             sheet,
                        adobe::name_t               bind,
                        adobe::name_t               bind_output,
                        adobe::name_t               action,
                        const adobe::any_regular_t& value,
                        const adobe::dictionary_t&  contributing)
{
    if (bind_output)
    {
        //touch(); // REVISIT (sparent) : We should have per item touch!
        sheet.set(bind_output, value);
        sheet.update();
    }
    else if (notifier)
    {
        if (bind)
        {
            adobe::dictionary_t result;
            result.insert(std::make_pair(adobe::key_value, value));
            result.insert(std::make_pair(adobe::key_contributing, adobe::any_regular_t(contributing)));
            notifier(action, adobe::any_regular_t(result));
        }
        else
        {
            notifier(action, value);
        }
    }
}

/*************************************************************************************************/

template <typename Cont>
void state_set_push_back(Cont& state_set, const adobe::factory_token_t& token, const button_item_t& temp)
{
    state_set.push_back(adobe::button_state_descriptor_t());

    state_set.back().name_m         = temp.name_m;
    state_set.back().alt_text_m     = temp.alt_text_m;
    state_set.back().modifier_set_m = temp.modifier_set_m;
    state_set.back().hit_proc_m     = boost::bind(&proxy_button_hit, token.notifier_m,
                                                    boost::ref(token.sheet_m), temp.bind_m,
                                                    temp.bind_output_m,
                                                    temp.action_m, _1, _2);
    state_set.back().value_m        = temp.value_m;
    state_set.back().contributing_m = temp.contributing_m;
}

/****************************************************************************************************/

void connect_button_state(adobe::button_t&          control,
                          adobe::assemblage_t&      assemblage,
                          adobe::sheet_t&           sheet,
                          const button_item_t&      temp,
                          adobe::eve_client_holder& /*client_holder*/)
{
    if (!temp.bind_m)
        return;

    /*
        REVISIT (sparent) : Don't currently propogate modifier mask past this point.
        Not yet wired up.
    */

    adobe::display_compositor_t<adobe::button_t, adobe::modifiers_t>*
        current_display(adobe::make_display_compositor(control, temp.modifier_set_m));

    assemblage.cleanup(boost::bind(adobe::delete_ptr<adobe::display_compositor_t<adobe::button_t, adobe::modifiers_t>*>(), current_display));
    assemblage.cleanup(boost::bind(&boost::signals::connection::disconnect, 
                                    sheet.monitor_invariant_dependent(
                                     temp.bind_m, boost::bind(&adobe::button_t::enable, 
                                                               boost::ref(control), _1))));

    adobe::attach_view(assemblage, temp.bind_m, *current_display, sheet);
    
    
    /*
        REVISIT (sparent) : Filtering the contributing code here isn't necessarily the right thing -
        I think monitor_contributing should remove the filter functionality if possible. For
        now I'm passing in an empty dictionary for no filtering.
    */

    assemblage.cleanup(boost::bind(&boost::signals::connection::disconnect, 
                                    sheet.monitor_contributing(temp.bind_m, adobe::dictionary_t(), 
                                                               boost::bind(&adobe::button_t::set_contributing, 
                                                                   boost::ref(control), 
                                                                   temp.modifier_set_m, _1))));
}

/****************************************************************************************************/

} // namespace

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

namespace implementation {

/****************************************************************************************************/

button_t* create_button_widget(const dictionary_t&    parameters,
                               const factory_token_t& token,
                               size_enum_t            size);

button_t* create_button_widget(const dictionary_t&    parameters,
                               const factory_token_t& token,
                               size_enum_t            size)
{
    bool               is_cancel(false);
    bool               is_default(false);
    modifiers_t        modifier_mask(modifiers_none_s);
    button_state_set_t state_set;
    array_t            items;
    button_item_t      item;

    get_value(parameters, key_name,        item.name_m);
    get_value(parameters, key_alt_text,    item.alt_text_m);
    get_value(parameters, key_bind,        item.bind_m);
    get_value(parameters, key_bind_output, item.bind_output_m);
    get_value(parameters, key_action,      item.action_m);
    get_value(parameters, key_value,       item.value_m);
    get_value(parameters, key_items,       items);
    get_value(parameters, key_default,     is_default);
    get_value(parameters, key_cancel,      is_cancel);

    for (array_t::const_iterator iter(items.begin()), last(items.end()); iter != last; ++iter)
        state_set_push_back(state_set, token, button_item_t(item, iter->cast<dictionary_t>()));

    bool state_set_originally_empty(state_set.empty());

    if (state_set_originally_empty)
        state_set_push_back(state_set, token, item);

    for (button_state_set_t::const_iterator first(state_set.begin()), last(state_set.end()); first != last; ++first)
        modifier_mask |= first->modifier_set_m;

    button_state_descriptor_t* first_state(state_set.empty() ? 0 : &state_set[0]);
    std::size_t                n(state_set.size());

    button_t* result = new button_t(is_default, is_cancel, modifier_mask,
                                    first_state, first_state + n,
                                    implementation::size_to_theme(size));

    for (array_t::const_iterator iter(items.begin()), last(items.end()); iter != last; ++iter)
    {
        button_item_t temp(item, iter->cast<dictionary_t>());

        connect_button_state(*result,
                             token.client_holder_m.assemblage_m,
                             token.sheet_m,
                             temp,
                             token.client_holder_m);
    }

    if (state_set_originally_empty)
    {
        connect_button_state(*result,
                             token.client_holder_m.assemblage_m,
                             token.sheet_m,
                             item,
                             token.client_holder_m);
    }

    return result;
}

/****************************************************************************************************/

} // namespace implementation

/****************************************************************************************************/

widget_node_t make_button(const dictionary_t&     parameters, 
                          const widget_node_t&    parent, 
                          const factory_token_t&  token,
                          const widget_factory_t& factory)
{ 
    size_enum_t   size(parameters.count(key_size) ?
                       implementation::enumerate_size(get_value(parameters, key_size).cast<name_t>()) :
                       parent.size_m);

    button_t* widget = implementation::create_button_widget(parameters, token, size);
    token.client_holder_m.assemblage_m.cleanup(boost::bind(delete_ptr<button_t*>(),widget));
   
    //
    // Call display_insertion to embed the new widget within the view heirarchy
    //
    platform_display_type display_token(insert(get_main_display(), parent.display_token_m, *widget));

    // set up key handler code. We do this all the time because we want the button to be updated
    // when modifier keys are pressed during execution of the dialog.

    keyboard_t::iterator keyboard_token(keyboard_t::get().insert(parent.keyboard_token_m, poly_key_handler_t(boost::ref(*widget))));
    
    
    //
    // As per SF.net bug 1428833, we want to attach the poly_placeable_t
    // to Eve before we attach the controller and view to the model
    //

    eve_t::iterator eve_token;
    eve_token = attach_placeable<poly_placeable_t>(parent.eve_token_m, *widget, parameters, 
        token, factory.is_container(static_name_t("button")),
        factory.layout_attributes(static_name_t("button")));

    //
    // Return the widget_node_t that comprises the tokens created for this widget by the various components
    //
    return widget_node_t(size, eve_token, display_token, keyboard_token);
}

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/
