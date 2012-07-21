/*
        Copyright 2008 Adobe Systems Incorporated
        Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
        or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_SHEET_HOOKS
#define ADOBE_SHEET_HOOKS

/*************************************************************************************************/

#include <GG/adobe/adam.hpp>
#include <GG/adobe/basic_sheet.hpp>
#include <GG/adobe/widget_proxies.hpp>
#include <GG/adobe/dictionary.hpp>

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/
/*!
    Hooks an object that models the View concept to a specified cell in a
    property model. The assemblage is necessary to pool the heap-allocated
    memory used to make the glue between your object and the property model.
    Once this routine is complete your object will behave as a view of the
    specified cell of the property model passed.
*/
template <typename Sheet, typename View>
void attach_view_to_model(adobe::assemblage_t& assemblage,
                          Sheet&               model,
                          adobe::name_t        cell,
                          View&                view)
{
    adobe::poly_view_t* poly_view(new adobe::poly_view_t(boost::ref(view)));

    assemblage_cleanup_ptr(assemblage, poly_view);

        adobe::sheet_t::monitor_value_t m(boost::bind(&adobe::ViewConcept<adobe::poly_view_t>::display,
                                                      boost::ref(*poly_view), _1));

        typename Sheet::connection_t c(model.monitor_value(cell, m));

        assemblage_cleanup_connection(assemblage, c);
}

/*************************************************************************************************/
/*!
    A simple routine that does several things:
        - Wraps the result coming out of a property controller into an
          adobe::any_regular_t
        - Pushes that value into the basic sheet at the cell specified
*/
template <typename T>
inline void forward_to_model(adobe::basic_sheet_t& sheet,
                             adobe::name_t         cell,
                             const T&              value)
{
    sheet.set(cell, adobe::any_regular_t(value));
}

/*************************************************************************************************/
/*!
    A simple routine that does several things:
        - Wraps the result coming out of a property controller into an
          adobe::any_regular_t
        - Pushes that value into the property model at the cell specified
        - Updates the property model, causing the newly-pushed value to
          propagate.
*/
template <typename T>
inline void forward_to_model(adobe::sheet_t& property_model,
                             adobe::name_t   cell,
                             const T&        value)
{
    property_model.set(cell, adobe::any_regular_t(value));
    property_model.update();
}

/*************************************************************************************************/
/*!
    Hooks an object that models the Controller concept to a specified cell in a
    property model. The assemblage is necessary to pool the heap-allocated
    memory used to make the glue between your object and the property model.
    Once this routine is complete your object will behave as a controller of the
    specified cell of the property model passed.
*/
template <typename Controller>
void attach_controller_to_model(adobe::assemblage_t& assemblage,
                                adobe::sheet_t&      property_model,
                                adobe::name_t        cell,
                                Controller&          controller)
{
    typedef typename adobe::ControllerConcept<Controller>::model_type controller_model_type;

    void (*forward_proc)(adobe::sheet_t&, adobe::name_t, const controller_model_type&) =
        &forward_to_model<controller_model_type>;

    controller.monitor(boost::bind(forward_proc, boost::ref(property_model), cell, _1));

    adobe::sheet_t::connection_t connection =
        property_model.monitor_enabled(cell, NULL, NULL,
                                       boost::bind(&Controller::enable, boost::ref(controller), _1));

    assemblage_cleanup_connection(assemblage, connection);
}

/*************************************************************************************************/
/*!
    Hooks an object that models the Controller concept to a specified cell in a
    basic sheet. The assemblage is necessary to pool the heap-allocated
    memory used to make the glue between your object and the basic sheet.
    Once this routine is complete your object will behave as a controller of the
    specified cell of the basic sheet passed.
*/
template <typename Controller>
void attach_controller_to_model(adobe::assemblage_t&  /*assemblage*/,
                                adobe::basic_sheet_t& model,
                                adobe::name_t         cell,
                                Controller&           controller)
{
    typedef typename adobe::ControllerConcept<Controller>::model_type controller_model_type;

    void (*forward_proc)(adobe::basic_sheet_t&, adobe::name_t, const controller_model_type&) =
        &forward_to_model<controller_model_type>;

    controller.monitor(boost::bind(forward_proc, boost::ref(model), cell, _1));
    controller.enable(true);
}

/*************************************************************************************************/
#if 0
#pragma mark -
#endif
/*************************************************************************************************/
/*!
    Hooks an object that models the View concept to a specified cell in a
    property model. The assemblage is necessary to pool the heap-allocated
    memory used to make the glue between your object and the property model.
    Once this routine is complete your object will behave as a view of the
    specified cell of the property model passed.
*/
template <typename ArgumentType, typename Sheet, typename Function>
inline void attach_view_function_to_model(adobe::assemblage_t& assemblage,
                                          Sheet&               model,
                                          adobe::name_t        cell,
                                          const Function&      function)
{
    adobe::poly_view_t* view(new adobe::poly_view_t(adobe::make_function_as_poly_view<ArgumentType>(function)));

    assemblage_cleanup_ptr(assemblage, view);

    attach_view_to_model(assemblage, model, cell, *view);
}

/*************************************************************************************************/
/*!
    Hooks two routines that together model the Controller concept to a property
    model.
*/
template <typename T,
          typename Sheet,
          typename MonitorFunction,
          typename EnableFunction>
inline void attach_controller_functions_to_model(adobe::assemblage_t&   assemblage,
                                                 Sheet&                 model,
                                                 adobe::name_t          cell,
                                                 const MonitorFunction& monitor_function,
                                                 const EnableFunction&  enable_function)
{
    adobe::poly_controller_t* controller(new adobe::poly_controller_t(adobe::make_functions_as_poly_controller<T>(monitor_function, enable_function)));

    assemblage_cleanup_ptr(assemblage, controller);

    attach_controller_to_model(assemblage, model, cell, *controller);
}

/*************************************************************************************************/

/*!
    \brief Create an adapter to allow a set of function-like objects to model
           both ViewConcept and ControllerConcept.

    This is useful in the rare cases when you have a widget that behaves as a
    widget for two or more unrelated values. (e.g., the angle/altitude picker
    widget).

    This routine takes three callbacks: the display()-equivalent,
    monitor()-equivalent and enable()-equivalent routines. All information
    between the model and the actual widget is forwarded through the proxy
    widget implementation unaltered.
*/

template <typename T,
          typename Sheet,
          typename DisplayFunction,
          typename MonitorFunction,
          typename EnableFunction>
inline void attach_widget_proxies_to_model(adobe::assemblage_t&   assemblage,
                                           Sheet&                 model,
                                           adobe::name_t          cell,
                                           const DisplayFunction& display_function,
                                           const MonitorFunction& monitor_function,
                                           const EnableFunction&  enable_function)
{
    attach_view_function_to_model<T>(assemblage, model, cell, display_function);
    attach_controller_functions_to_model<T>(assemblage, model, cell, monitor_function, enable_function);
}


//Attach only the view to a funtion
        
template <typename T ,
                  typename Function, 
                  typename FactoryToken>
void attach_view_function_direct(const FactoryToken&    token,
                                                                 const name_t                   cell,
                                                                 const Function&                function )
{
        basic_sheet_t& layout_sheet(*token.mEveViewHolder.mLayoutSheet);
        // is the cell in the layout sheet or the Adam sheet?
        if (layout_sheet.count_interface(cell) != 0)
        {
                        
                attach_view_function_to_model<T>(token.mEveViewHolder.GetAssemblage(),
                                                                                 layout_sheet,                          
                                                                                 cell,
                                                                                 function);     
        }
        else
        {
                attach_view_function_to_model<T>(token.mEveViewHolder.GetAssemblage(),
                                                                                 *token.mSheet,                         
                                                                                 cell,
                                                                                 function);     
        }
}
        
template <typename T, 
                  typename Function,
                  typename FactoryToken>
void attach_view_function(const adobe::dictionary_t&    parameters,
                                                  const FactoryToken&                   token,
                                                  const adobe::name_t                   key_name,
                                                  const Function&                               function )
{
        if (parameters.count(key_name) 
                && get_value(parameters, key_name).type_info() == adobe::type_info<name_t>())
        {
                name_t         cell(get_value(parameters,key_name).cast<name_t>());             
                
                attach_view_function_direct<T>(token,cell,function);
        }
}
        
        
        
        
/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/
// ADOBE_SHEET_HOOKS
#endif
/*************************************************************************************************/
