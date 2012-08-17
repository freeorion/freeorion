/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_DEBOUNCE_HPP
#define ADOBE_DEBOUNCE_HPP

/*************************************************************************************************/

#include <GG/adobe/config.hpp>

#include <boost/function.hpp>

#include <GG/adobe/any_regular.hpp>

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/

/*!
    \ingroup apl_libraries

    \brief Utility class to short-circuit feedback issues with model-view-controller systems

    When a "widget" models both a view and a controller, there can be issues of feedback loops.
    "Debouncing" is the process of detecting and short-circuiting these feedback loops. The loops
    have been broken into two types.

    Type 1 feedback loops take place when the controller (typically owned by the OS) originates
    a change (presumably by manual user intervention). This change is propagated to the model as
    a request to change the value of a cell. The model, in turn, should then notify the view of
    the new cell value that it should display. Since the view is also the controller, we need
    to short-circuit at the point when the value comes back to the widget from the model. Since
    the controller's value may be altered by the model (which is the component carrying the state
    of the system) we track the value as it is being sent to the model and compare it going back
    out to the display.

    Type 2 feedback takes place when the model is poked by a third-party controller, requiring
    the view of this widget to display something new. The model notifies the view, which then
    relays the information to the *actual* view, typically a control owned by the OS. Depending
    on the OS (Carbon being most notorious) the control may recieve back notification that its
    value has changed. Since the controller is also the view, we need to short-circuit at the
    point when the value comes back to the widget fromt the OS. Since the view is stateless
    within the system we do not care about the value being relayed by the OS to the controller.
    In this instance, then, using a flag to track type 2 feedback is sufficient.
*/

template <typename ModeType>    
class debounce_t
{
public:
    typedef ModeType                                  model_type;
    typedef boost::function<void (const model_type&)> proc_type;

    debounce_t() :
        type_2_debounce_m(false),
        first_time_m(true)
    { }

    /*!
        /param set_model_proc This is the proc that will be called when the model needs
                              to be notified that the controller has changed to a new
                              value. This proc should point to the model.

        /param monitor_model_proc This is the proc that will be called when the view needs
                                  to be notified that the model's cell has changed to a
                                  new value. This proc should point to the widget (OS).
    */

    template <typename UnaryFunction1, typename UnaryFunction2>
    debounce_t(const UnaryFunction1& set_model_proc, const UnaryFunction2& monitor_model_proc) :
        set_model_m(set_model_proc),
        monitor_model_m(monitor_model_proc),
        type_2_debounce_m(false),
        first_time_m(true)
    { }

    /*!
        Intended to be called by the widget (OS) whenever the value is changed (by either
        the user or if the OS chooses to feeedback). The proc will deduce the originator
        by retained internal state, and will opt to propagate the value to the model when
        appropriate.
    */
    void set_model_value(const model_type& from_controller)
    {
        assert (set_model_m);

        if (first_time_m)
            type_2_debounce_m = false;

        if (type_2_debounce_m)
        {
            type_2_debounce_m = false;

            return;
        }

        type_1_debounce_m = from_controller;

        set_model_m(from_controller);

        first_time_m = false;
    }

    /*!
        Intended to be called by the model whenever the value is changed. The proc will
        deduce the originator by retained internal state, and will opt to propagate the
        message to the control (OS) when appropriate.
    */
    void display(const model_type& new_value)
    {
        assert (monitor_model_m);

        if (new_value == type_1_debounce_m)
            return;
        
        type_1_debounce_m = new_value;
        type_2_debounce_m = true;

        monitor_model_m(new_value);
    }

    void reset_set_model_proc(const proc_type& proc)
    { type_1_debounce_m = model_type(); set_model_m = proc; }

    void reset_monitor_model_proc(const proc_type& proc)
    { type_2_debounce_m = false; monitor_model_m = proc; }

    const model_type& last() const
    { return type_1_debounce_m; }

    bool is_set_model_proc_installed() const
    { return set_model_m; }

    bool is_monitor_model_proc_installed() const
    { return monitor_model_m; }

private:
    proc_type  set_model_m;
    proc_type  monitor_model_m;
    model_type type_1_debounce_m;
    bool       type_2_debounce_m;
    bool       first_time_m;
};

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/

// ADOBE_DEBOUNCE_HPP
#endif

/*************************************************************************************************/
