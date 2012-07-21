/*
    Copyright 2006-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_POLY_SEQUENCE_CONTROLLER_HPP
#define ADOBE_POLY_SEQUENCE_CONTROLLER_HPP

/*************************************************************************************************/

#include <GG/adobe/config.hpp>

#include <boost/function.hpp>

#include <GG/adobe/poly.hpp>
#include <GG/adobe/sequence_controller.hpp>
#include <GG/adobe/poly_sequence_model.hpp>
#include <GG/adobe/vector.hpp>

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/
/*!
    \ingroup sequence_mvc

    \brief poly_ holder for implementations that model the SequenceController concept.
*/
/*************************************************************************************************/
/*!
    \ingroup sequence_mvc

    \brief poly_ concept for implementations that model the SequenceController concept.
*/
template <typename T>
struct poly_sequence_controller_interface : poly_copyable_interface
{
    virtual void monitor_sequence(typename poly_sequence_model<T>::type&) = 0;
};

/*************************************************************************************************/
/*!
    \ingroup sequence_mvc

    \brief poly_ instance for implementations that model the SequenceController concept.
*/
template <typename T>
struct poly_sequence_controller_instance 
{
    template <typename V>
    struct type : optimized_storage_type<V, poly_sequence_controller_interface<T> >::type
    {
        typedef typename optimized_storage_type<V, poly_sequence_controller_interface<T> >::type base_t;

        BOOST_CLASS_REQUIRE(V, adobe, SequenceControllerConcept);

        explicit type(const V& x) :
            base_t(x)
        { }

        type(move_from<type> x) :
            base_t(move_from<base_t>(x.source))
        { }

        void monitor_sequence(typename poly_sequence_model<T>::type& sequence)
        { SequenceControllerConcept<V>::monitor_sequence(this->get(), sequence); }
    };
};

/*************************************************************************************************/
/*!
    \ingroup sequence_mvc

    \brief poly_ holder for implementations that model the SequenceController concept.
*/
template <typename T>
struct sequence_controller : poly_base<poly_sequence_controller_interface<T>,
                                       poly_sequence_controller_instance<T>::template type>
{
    typedef poly_base<poly_sequence_controller_interface<T>,
                      poly_sequence_controller_instance<T>::template type> base_t;

    typedef T value_type;

    template <typename V>
    explicit sequence_controller(const V& s) :
        base_t(s)
    { }

    sequence_controller(move_from<sequence_controller> x) :
        base_t(move_from<base_t>(x.source))
    { }

    void monitor_sequence(typename poly_sequence_model<T>::type& sequence)
    { this->interface_ref().monitor_sequence(sequence); }
};

/*************************************************************************************************/

template <typename T>
struct poly_sequence_controller
{
    typedef poly< sequence_controller<T> > type;
};

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/

// ADOBE_POLY_SEQUENCE_CONTROLLER_HPP
#endif 

/*************************************************************************************************/
