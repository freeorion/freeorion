/*
    Copyright 2008 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_WIDGET_PROXIES_HPP
#define ADOBE_WIDGET_PROXIES_HPP

/*************************************************************************************************/

#include <GG/adobe/config.hpp>

#include <GG/adobe/future/assemblage.hpp>
#include <GG/adobe/poly_controller.hpp>
#include <GG/adobe/poly_view.hpp>

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/

namespace implementation {

/*************************************************************************************************/
/*!
*/
template <typename T,
          typename MonitorFunction,
          typename EnableFunction>
struct functions_as_controller
{
    typedef T                                         model_type;
    typedef boost::function<void (const model_type&)> setter_type;

    functions_as_controller(const MonitorFunction& monitor_function,
                            const EnableFunction&  enable_function) :
        monitor_function_m(monitor_function),
        enable_function_m(enable_function)
    { }

    void monitor(const setter_type& proc)
    {
        monitor_function_m(proc);
    }

    void enable(bool enable)
    {
        enable_function_m(enable);
    }

    friend inline bool operator==(const functions_as_controller&, const functions_as_controller&)
    { return true; }

private:
    MonitorFunction monitor_function_m;
    EnableFunction  enable_function_m;
};

/*************************************************************************************************/

template <typename T, typename MF, typename EF>
inline functions_as_controller<T, MF, EF> make_functions_as_controller(const MF& mf,
                                                                       const EF& ef)
{
    return functions_as_controller<T, MF, EF>(mf, ef);
}

/*************************************************************************************************/

} // namespace implementation

/*************************************************************************************************/

/*!
    \brief Create an adapter to allow a pair of function-like objects to model ControllerConcept.
*/

template <typename T,
          typename MonitorFunction,
          typename EnableFunction>
inline poly<controller> make_functions_as_poly_controller(const MonitorFunction& monitor_function,
                                                          const EnableFunction&  enable_function)
{
    return poly<controller>(implementation::functions_as_controller<T,
                                                                    MonitorFunction,
                                                                    EnableFunction>(monitor_function,
                                                                                    enable_function));
}

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/
// ADOBE_WIDGET_PROXIES_HPP
#endif

/*************************************************************************************************/
