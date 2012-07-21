/*
    Copyright 2006-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_POLY_KEY_HANDLER_HPP
#define ADOBE_POLY_KEY_HANDLER_HPP

/*************************************************************************************************/

#include <GG/adobe/config.hpp>

#include <GG/adobe/poly.hpp>
#include <GG/adobe/key_handler_concept.hpp>
#include <GG/adobe/future/platform_primitives.hpp>
#include <GG/adobe/widget_attributes.hpp>

#include <boost/ref.hpp>
#include <boost/operators.hpp>

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/

namespace implementation {

/*************************************************************************************************/
/*!
    deref returns a ref to the underlying value in the case that T is a boost::reference_wrapper
    otherwise it is the identity function. This overload handles the non boost::reference_wrapper
    case.
*/
template <class T>
inline
typename boost::disable_if<boost::is_reference_wrapper<T>, T& >::type
deref(T& t) 
{
    return t;
}

/*************************************************************************************************/
/*!
    deref returns a ref to the underlying value in the case that T is a boost::reference_wrapper
    otherwise it is the identity function. This overload handles the boost::reference_wrapper case.
*/
template <class T>
inline
typename boost::enable_if<boost::is_reference_wrapper<T>, 
                          typename boost::unwrap_reference<T>::type& >::type
deref(T& t) 
{
    return t.get();
}
/*************************************************************************************************/

} //namespace implementation

/*************************************************************************************************/

struct poly_key_handler_interface : poly_copyable_interface
{
    virtual bool handle_key(key_type key, bool pressed, modifiers_t modifiers) = 0;
    virtual any_regular_t underlying_handler() = 0;
};


template <typename T>
struct poly_key_handler_instance : optimized_storage_type<T, poly_key_handler_interface>::type
{
    typedef typename optimized_storage_type<T, poly_key_handler_interface>::type base_t;

    typedef typename boost::unwrap_reference<T>::type deref_type;
    
    BOOST_CLASS_REQUIRE(deref_type, adobe, KeyHandlerConcept);

    poly_key_handler_instance(const T& x) 
        : base_t(x) {}

    poly_key_handler_instance(move_from<poly_key_handler_instance> x) 
        : base_t(move_from<base_t>(x.source)) {}

    bool handle_key(key_type key, bool pressed, modifiers_t modifiers)
    { 
        using adobe::handle_key;
        return handle_key(implementation::deref(this->get()), key, pressed, modifiers); 
    }

    any_regular_t underlying_handler()
    { 
        using adobe::underlying_handler;
        return underlying_handler(implementation::deref(this->get())); 
    }
};

/*************************************************************************************************/

struct key_handler : public poly_base<poly_key_handler_interface, poly_key_handler_instance>
{
    typedef poly_base<poly_key_handler_interface, poly_key_handler_instance> base_t;

    template <typename T>
    explicit key_handler(const T& s) : base_t(s) { }

    key_handler(move_from<key_handler> x) : base_t(move_from<base_t>(x.source)) {}

    bool handle_key(key_type key, bool pressed, modifiers_t modifiers)
    { return interface_ref().handle_key(key, pressed, modifiers); }

    any_regular_t underlying_handler()
    { return interface_ref().underlying_handler(); }
};

/*************************************************************************************************/

typedef poly<key_handler> poly_key_handler_t;

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/

#endif
