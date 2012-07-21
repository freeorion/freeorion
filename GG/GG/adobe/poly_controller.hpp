/*
    Copyright 2006-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_POLY_CONTROLLER_HPP
#define ADOBE_POLY_CONTROLLER_HPP

/*************************************************************************************************/

#include <GG/adobe/config.hpp>

#include <GG/adobe/functional.hpp>
#include <GG/adobe/poly.hpp>
#include <GG/adobe/any_regular.hpp>
#include <GG/adobe/controller_concept.hpp>

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/

namespace implementation {

/*************************************************************************************************/

template <class T>
inline boost::function<void (const T&)> 
make_setter(const boost::function<void (const any_regular_t&)>& f) {
    return boost::bind(f, boost::bind(constructor<any_regular_t>(), _1));
}

/*************************************************************************************************/

}
    
/*************************************************************************************************/

struct poly_controller_interface : public poly_copyable_interface
{
    virtual void monitor(boost::function<void (const any_regular_t&)> setter) = 0;
    virtual void enable(bool enable_state) = 0;
};

/*************************************************************************************************/

template <typename T>
struct poly_controller_instance : optimized_storage_type<T, poly_controller_interface>::type
{ 
    typedef typename optimized_storage_type<T, poly_controller_interface>::type base_t;

    BOOST_CLASS_REQUIRE(T, adobe, ControllerConcept);

    poly_controller_instance(const T& x) : base_t(x) {}
    poly_controller_instance(move_from<poly_controller_instance> x) 
        : base_t(move_from<base_t>(x.source)) {}

    void monitor(boost::function<void (const any_regular_t&)> f)
    { 
        ControllerConcept<T>::monitor(this->get(),
            implementation::make_setter<typename ControllerConcept<T>::model_type>(f));
    }

    void enable(bool enable_state)
    { 
      ControllerConcept<T>::enable(this->get(), enable_state); 
    }
};


/*************************************************************************************************/


struct controller : poly_base<poly_controller_interface, poly_controller_instance>
{
    typedef poly_base<poly_controller_interface, poly_controller_instance> base_t;

    template <typename T> // T Models Controller
    explicit controller(const T& s) :  base_t(s) { }

    controller(move_from<controller> x) : base_t(move_from<base_t>(x.source)) {}

    void monitor(const boost::function<void (const any_regular_t&)>& f)
    { interface_ref().monitor(f); }

    void enable(bool enable_state)
    { interface_ref().enable(enable_state); }

};

/*************************************************************************************************/

typedef poly<controller> poly_controller_t;

/*************************************************************************************************/

template <>
struct controller_model_type<poly_controller_t>
{
    typedef any_regular_t type;
};


} // namespace adobe

/*************************************************************************************************/

#endif 
