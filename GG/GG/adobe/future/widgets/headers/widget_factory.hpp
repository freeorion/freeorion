/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_WIDGET_FACTORY_HPP
#define ADOBE_WIDGET_FACTORY_HPP

/*************************************************************************************************/

#include <boost/concept_check.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits.hpp>
#include <boost/mpl/bool_fwd.hpp>
#include <boost/signals/connection.hpp>

#include <GG/adobe/adam.hpp>
#include <GG/adobe/any_regular.hpp>
#include <GG/adobe/basic_sheet.hpp>
#include <GG/adobe/eve.hpp>
#include <GG/adobe/eve_evaluate.hpp>
#include <GG/adobe/poly_controller.hpp>
#include <GG/adobe/poly_placeable.hpp>
#include <GG/adobe/poly_view.hpp>

#include <GG/adobe/future/widgets/headers/factory.hpp>
#include <GG/adobe/future/widgets/headers/widget_tokens.hpp>

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/

typedef std::vector<name_t> touch_set_t;

struct factory_token_t;

/*************************************************************************************************/

template <typename FactoryToken>
basic_sheet_t& token_layout_sheet(const FactoryToken& token)
{
    return token.client_holder_m.layout_sheet_m;
}

/*************************************************************************************************/

template <typename FactoryToken>
sheet_t& token_property_model(const FactoryToken& token)
{
    return token.sheet_m;
}

/*************************************************************************************************/

template <typename FactoryToken>
assemblage_t& token_assemblage(const FactoryToken& token)
{
    return token.client_holder_m.assemblage_m;
}

/*************************************************************************************************/

template <typename FactoryToken>
behavior_t& token_layout_behavior(const FactoryToken& token)
{
    return token.client_holder_m.root_behavior_m;
}

/****************************************************************************************************/

namespace implementation {

/*************************************************************************************************/

inline void sheet_set(sheet_t*             sheet,
                      name_t               name,
                      const any_regular_t& value)
{
    sheet->set(name, value);
    sheet->update();
}

/*************************************************************************************************/

inline void layout_sheet_set(basic_sheet_t*       sheet,
                             name_t               name,
                             const any_regular_t& value)
{
    sheet->set(name, value);
}

/*************************************************************************************************/

size_enum_t enumerate_size(const name_t& size);

/*************************************************************************************************/

theme_t size_to_theme(size_enum_t size);

/*************************************************************************************************/

touch_set_t touch_set(const dictionary_t& parameters);

/*************************************************************************************************/

template <typename Sheet, typename ModelType>
inline typename boost::enable_if<boost::is_same<Sheet, basic_sheet_t>, void>::type
    touch_set_update(Sheet&                          /*sheet*/,
                     const sheet_t::monitor_value_t& set_proc,
                     const ModelType&                new_value,
                     touch_set_t                     /*touch_set*/,
                     behavior_t&                     layout_behavior)
{
    set_proc(any_regular_t(new_value));

    layout_behavior();
}

/*************************************************************************************************/

template <typename Sheet, typename ModelType>
inline typename boost::disable_if<boost::is_same<Sheet, basic_sheet_t>, void>::type
    touch_set_update(Sheet&                          sheet,
                     const sheet_t::monitor_value_t& set_proc,
                     const ModelType&                new_value,
                     touch_set_t                     touch_set,
                     behavior_t&                     layout_behavior)
{
    // If the cell is not bound to the Adam sheet, but rather the layout sheet,
    // then should this step be taking place?
    if (!touch_set.empty())
        sheet.touch(&touch_set.front(), &touch_set.front() + touch_set.size());

    set_proc(any_regular_t(new_value));

    layout_behavior();
}

/*************************************************************************************************/

template <typename Sheet>
inline typename boost::enable_if<boost::is_same<Sheet, basic_sheet_t>, 
                                 sheet_t::monitor_value_t>::type
    obtain_set_cell_from_controller_proc(Sheet&        sheet,
                                         name_t cell)
{
    return boost::bind(&implementation::layout_sheet_set,
                       &sheet,
                       cell,
                       _1);
}

/*************************************************************************************************/

template <typename Sheet>
inline typename boost::disable_if<boost::is_same<Sheet, basic_sheet_t>, 
                                  sheet_t::monitor_value_t>::type
    obtain_set_cell_from_controller_proc(Sheet&        sheet,
                                         name_t cell)
{
    return boost::bind(&implementation::sheet_set,
                       &sheet,
                       cell,
                       _1);
}

/*************************************************************************************************/

} // namespace implementation

/*************************************************************************************************/

template <class AP, class P> // P models poly_placeable_t or poly_placeable_twopass_t, 
inline eve_t::iterator attach_placeable(eve_t::iterator     parent_token,
                                               P&                         widget,
                                               const dictionary_t&        parameters,
                                               const factory_token_t&     token,
                                               bool                       is_container,
                                               const layout_attributes_t& layout_attributes)
{
    AP* element = new AP(&widget);
    token.client_holder_m.assemblage_m.cleanup(boost::bind(delete_ptr<AP*>(), element));
    
    layout_attributes_t attributes = layout_attributes;
    apply_layout_parameters(attributes, parameters);

    return token.client_holder_m.eve_m.add_placeable(   parent_token,                     
                                                        attributes,
                                                        is_container,
                                                        poly_cast<poly_placeable_t&>(*element));
}

/*************************************************************************************************/

template <typename Controller, typename Sheet, typename FactoryToken>
void attach_monitor(Controller&         controller, 
                    name_t              cell, 
                    Sheet&              sheet, 
                    const FactoryToken& token, 
                    const dictionary_t& parameters)
{
    sheet_t::monitor_value_t set_cell_from_controller_proc =
        implementation::obtain_set_cell_from_controller_proc(sheet, cell);

    typedef typename ControllerConcept<Controller>::model_type controller_model_type;

    boost::function<void (const controller_model_type&)> setter
        (boost::bind(&implementation::touch_set_update<Sheet, controller_model_type>,
                                                       boost::ref(sheet),
                                                       set_cell_from_controller_proc,
                                                       _1,
                                                       implementation::touch_set(parameters),
                                                       boost::ref(token_layout_behavior(token))));

     controller.monitor(setter);
}

/*************************************************************************************************/
/*!
    Behaves much like a typical controller would, except is meant to broadcast the new
    value from a controller to multiple sheet cells instead of just one. The cell set to
    broadcast to is sent in to the splitter_controller_adaptor as an array. Currently the
    class only supports the basic sheet type, not the property model sheet.
*/
template <typename ModelType>
class splitter_controller_adaptor
{
public:
    typedef ModelType model_type;

    splitter_controller_adaptor(basic_sheet_t& sheet,
                                behavior_t&    layout_behavior,
                                const array_t& cell_set,
                                modifiers_t    pass_thru = modifiers_t()) :
        sheet_m(&sheet),
        layout_behavior_m(&layout_behavior),
        cell_set_m(cell_set),
        pass_thru_m(pass_thru)
    { }

    void operator()(const model_type& value,
                    modifiers_t       modifiers = modifiers_t())
    {
        if ((pass_thru_m & modifiers) == 0)
            return;

        array_t::iterator iter(cell_set_m.begin());
        array_t::iterator last(cell_set_m.end());

        while (iter != last)
        {
            sheet_m->set(iter->cast<name_t>(), value);

            ++iter;
        }

        (*layout_behavior_m)();
    }

private:
    basic_sheet_t* sheet_m;
    behavior_t*    layout_behavior_m;
    array_t        cell_set_m;
    modifiers_t    pass_thru_m;
};

/*************************************************************************************************/

template <typename View>
struct force_relayout_view_adaptor
{
    typedef typename ViewConcept<View>::model_type model_type;

    force_relayout_view_adaptor(View& view, visible_change_queue_t& queue) :
        view_m(&view),
        queue_m(&queue)
    { }

    void display(const model_type& value)
    {
        assert(view_m && queue_m);

        ViewConcept<View>::display(*view_m, value);

        queue_m->force_m = true;
    }

    View*                   view_m;
    visible_change_queue_t* queue_m;
};

/*************************************************************************************************/

template <class View, class Sheet>
void attach_view(assemblage_t& assemblage,
                 name_t        cell,
                 View&         control,
                 Sheet&        sheet)
{
    poly_view_t* view(new poly_view_t(boost::ref(control)));
    assemblage_cleanup_ptr(assemblage, view);
        sheet_t::monitor_value_t m(boost::bind(&ViewConcept<poly_view_t>::display, 
                                           boost::ref(*view), _1));
        typename Sheet::connection_t c(sheet.monitor_value(cell, m));
        assemblage_cleanup_connection(assemblage, c);
}

/*************************************************************************************************/

template <class Controller, class Sheet>
typename boost::disable_if<boost::is_same<Sheet, basic_sheet_t>, void>::type
    attach_enabler(assemblage_t& assemblage, name_t cell, Controller& control, Sheet& sheet,
                    const dictionary_t& parameters)
{
    poly_controller_t* controller(new poly_controller_t(&control));
    assemblage_cleanup_ptr(assemblage, controller);
    touch_set_t v(implementation::touch_set(parameters));
        sheet_t::monitor_enabled_t 
        m(boost::bind(&ControllerConcept<poly_controller_t>::enable, boost::ref(*controller), _1));
        
    typename Sheet::connection_t c;
    if(v.empty()) c = sheet.monitor_enabled(cell, NULL, NULL, m);
    else c = sheet.monitor_enabled(cell, &v[0], &v[0] + v.size(), m);
        assemblage_cleanup_connection(assemblage, c);
}

/*************************************************************************************************/

template <class Controller, class Sheet>
inline typename boost::enable_if<boost::is_same<Sheet, basic_sheet_t>, void>::type
    attach_enabler(assemblage_t&, name_t, Controller&, Sheet&,  const dictionary_t&)
{ }

/*************************************************************************************************/

template <typename T, typename P>
inline widget_node_t create_and_hookup_widget(const dictionary_t& parameters,
                                       const widget_node_t&       parent,
                                       const factory_token_t&     token,
                                       bool                       is_container,
                                       const layout_attributes_t& layout_attributes)
{
    size_enum_t   size(parameters.count(key_size) ?
                       implementation::enumerate_size(
                           get_value(parameters, key_size).cast<name_t>()) : parent.size_m);

    T*              widget(NULL);
    create_widget(parameters, size, widget);
    token.client_holder_m.assemblage_m.cleanup(boost::bind(delete_ptr<T*>(),widget));
   
    //
    // Call display_insertion to embed the new widget within the view heirarchy
    //
    platform_display_type display_token(insert(get_main_display(), parent.display_token_m, *widget));

    //
    // As per SF.net bug 1428833, we want to attach the poly_placeable_t
    // to Eve before we attach the controller and view to the model
    //

    eve_t::iterator eve_token;

    eve_token = attach_placeable<P>(parent.eve_token_m,
                                           *widget,
                                           parameters,
                                           token,
                                           is_container,
                                           layout_attributes);

    attach_view_and_controller(*widget, parameters, token);

    //
    // Return the widget_node_t that comprises the tokens created for
    // this widget by the various components
    //
    //revist: MM, hook up keyboard insertion mechanism
    return widget_node_t(size, eve_token, display_token, parent.keyboard_token_m);
}

/****************************************************************************************************/

template <typename Controller, typename Sheet, typename FactoryToken>
void couple_controller_to_cell(Controller&           controller, 
                               name_t                cell, 
                               Sheet&                sheet, 
                               const FactoryToken&   token, 
                               const dictionary_t&   parameters)

{
    attach_enabler(token_assemblage(token), cell, controller, sheet, parameters);
    attach_monitor(controller, cell, sheet, token, parameters);
}

/*************************************************************************************************/

template <typename T, typename FactoryToken>
void attach_controller_direct(T&                                                        control,
                                                          const adobe::dictionary_t&    parameters,
                                                          const FactoryToken&                   token,
                                                          adobe::name_t                                 cell)
{
        if (cell == name_t())
                return;
        
        basic_sheet_t& layout_sheet(token_layout_sheet(token));
        sheet_t&       property_model(token_property_model(token));
        
        // is the cell in the layout sheet or the Adam sheet?
        if (layout_sheet.count_interface(cell) != 0)
                couple_controller_to_cell(control, cell, layout_sheet,
                                                                  token, parameters);
        else
                couple_controller_to_cell(control, cell, property_model,
                                                                  token, parameters);
}

/*************************************************************************************************/

template <typename T, typename FactoryToken>
void attach_controller(T&                                                 control,
                       const adobe::dictionary_t& parameters,
                       const FactoryToken&                token,
                       adobe::name_t                      key_name = key_bind)
{
    name_t cell;

    get_value(parameters, key_name, cell);
        
        attach_controller_direct(control,parameters,token,cell);
}

/*************************************************************************************************/

template <typename T, typename FactoryToken>
void attach_view_direct(T&                                                      control,
                                                const adobe::dictionary_t&      /*parameters*/,
                                                const FactoryToken&         token,
                                                adobe::name_t               cell)
{
        if (cell == name_t())
                return;
        
        basic_sheet_t& layout_sheet(token_layout_sheet(token));
        assemblage_t&  assemblage(token_assemblage(token));
        
        // is the cell in the layout sheet or the Adam sheet?
        if (layout_sheet.count_interface(cell) != 0)
                attach_view(assemblage, cell, control, layout_sheet);
        else
                attach_view(assemblage, cell, control, token_property_model(token));    
}
        
/*************************************************************************************************/

template <typename T, typename FactoryToken>
void attach_view(T&                                                     control,
                 const adobe::dictionary_t& parameters,
                 const FactoryToken&            token,
                 adobe::name_t                          key_name = key_bind)
{
    name_t cell;

    get_value(parameters, key_name, cell);

        attach_view_direct(control, parameters, token, cell);
}

/*************************************************************************************************/

template <typename T, typename FactoryToken>
void attach_view_and_controller_direct(T&                                                       control,
                                       const adobe::dictionary_t&       parameters,
                                       const FactoryToken&                      token,
                                       adobe::name_t                            widget_cell = name_t(),
                                       adobe::name_t                            view_cell = name_t(),
                                       adobe::name_t                            controller_cell = name_t())
{
        if (widget_cell == name_t() && view_cell== name_t() && controller_cell== name_t())
                return;

        basic_sheet_t& layout_sheet(token_layout_sheet(token));
    sheet_t&       property_model(token_property_model(token));
        
        if (widget_cell != name_t())
    {
        controller_cell = widget_cell;
        view_cell = widget_cell;
    }
        
    if (controller_cell != name_t())
    {
        // is the cell in the layout sheet or the Adam sheet?
                
        if (layout_sheet.count_interface(controller_cell) != 0)
            couple_controller_to_cell(control, controller_cell, layout_sheet, 
                                      token, parameters);
        else
            couple_controller_to_cell(control, controller_cell, property_model, 
                                      token, parameters);
    }
        
    if (view_cell != name_t())
    {
        // is the cell in the layout sheet or the Adam sheet?
                
        assemblage_t& assemblage(token_assemblage(token));
                
        if (layout_sheet.count_interface(view_cell) != 0)
            attach_view(assemblage, view_cell, control, layout_sheet);
        else
            attach_view(assemblage, view_cell, control, token_property_model(token));
    }
}

/*************************************************************************************************/

template <typename T, typename FactoryToken>
void attach_view_and_controller(T&                  control,
                                const dictionary_t& parameters,
                                const FactoryToken& token,
                                adobe::name_t       bind_cell_name = key_bind,
                                adobe::name_t       bind_view_cell_name = key_bind_view,
                                adobe::name_t       bind_controller_cell_name = key_bind_controller)
{
    if (parameters.count(bind_cell_name) == 0 &&
        parameters.count(bind_view_cell_name) == 0 &&
        parameters.count(bind_controller_cell_name) == 0)
        return;

    name_t controller_cell;
    name_t view_cell;
    name_t widget_cell;

    get_value(parameters, bind_controller_cell_name, controller_cell);
    get_value(parameters, bind_view_cell_name, view_cell);
    get_value(parameters, bind_cell_name, widget_cell);

        attach_view_and_controller_direct(control, parameters, token, widget_cell, view_cell, controller_cell);
}

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/

#endif

/*************************************************************************************************/
