/*
    Copyright 2006-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/******************************************************************************/

#ifndef ADOBE_POLY_SEQUENCE_VIEW_HPP
#define ADOBE_POLY_SEQUENCE_VIEW_HPP

/******************************************************************************/

#include <GG/adobe/config.hpp>

#include <boost/function.hpp>

#include <GG/adobe/copy_on_write.hpp>
#include <GG/adobe/poly.hpp>
#include <GG/adobe/sequence_model_fwd.hpp>
#include <GG/adobe/sequence_view.hpp>
#include <GG/adobe/vector.hpp>

/******************************************************************************/

namespace adobe {

/******************************************************************************/
/*!
    \ingroup sequence_mvc

    \brief poly_ concept for implementations that model the SequenceView concept.
*/
template <typename T>
struct poly_sequence_view_interface : poly_copyable_interface
{
    typedef copy_on_write<T> cow_value_type;

    virtual void refresh(sequence_key<T> index, cow_value_type value) = 0;

    virtual void extend(sequence_key<T> before, sequence_key<T> value_key, cow_value_type value) = 0;

    virtual void extend_set(sequence_key<T> before, const vector<sequence_key<T> >& extend_key_set) = 0;

    virtual void erase(const vector<sequence_key<T> >& key_set) = 0;

    virtual void clear() = 0;
};

/******************************************************************************/
/*!
    \ingroup sequence_mvc

    \brief poly_ instance for implementations that model the SequenceView concept.
*/
template <typename T>
struct poly_sequence_view_instance
{
    typedef typename poly_sequence_view_interface<T>::cow_value_type cow_value_type;

    template <typename V>
    struct type : optimized_storage_type<V, poly_sequence_view_interface<T> >::type
    {
        typedef typename optimized_storage_type<V, poly_sequence_view_interface<T> >::type base_t;

        BOOST_CLASS_REQUIRE(V, adobe, SequenceViewConcept);

        explicit type(const V& x) :
            base_t(x)
        { }

        type(move_from<type> x) :
            base_t(move_from<base_t>(x.source))
        { }

        void refresh(sequence_key<T> index, cow_value_type value)
        { SequenceViewConcept<V>::refresh(this->get(), index, value); }
    
        void extend(sequence_key<T> before, sequence_key<T> value_key, cow_value_type value)
        { SequenceViewConcept<V>::extend(this->get(), before, value_key, value); }

        void extend_set(sequence_key<T> before, const vector<sequence_key<T> >& extend_key_set)
        { SequenceViewConcept<V>::extend_set(this->get(), before, extend_key_set); }

        void erase(const vector<sequence_key<T> >& key_set)
        { SequenceViewConcept<V>::erase(this->get(), key_set); }

        void clear()
        { SequenceViewConcept<V>::clear(this->get()); }
    };
};

/******************************************************************************/
/*!
    \ingroup sequence_mvc

    \brief poly_ holder for implementations that model the SequenceView concept.
*/
template <typename T>
struct sequence_view : poly_base<poly_sequence_view_interface<T>,
                                 poly_sequence_view_instance<T>::template type>
{
    typedef poly_base<poly_sequence_view_interface<T>,
                      poly_sequence_view_instance<T>::template type> base_t;

    typedef typename poly_sequence_view_instance<T>::cow_value_type cow_value_type;

    template <typename V>
    explicit sequence_view(const V& s) :
        base_t(s)
    { }

    sequence_view(move_from<sequence_view> x) :
        base_t(move_from<base_t>(x.source))
    { }

    void refresh(sequence_key<T> index, cow_value_type value)
    { this->interface_ref().refresh(index, value); }

    void extend(sequence_key<T> before, sequence_key<T> value_key, cow_value_type value)
    { this->interface_ref().extend(before, value_key, value); }

    void extend_set(sequence_key<T> before, const vector<sequence_key<T> >& extend_key_set)
    { this->interface_ref().extend_set(before, extend_key_set); }

    void erase(const vector<sequence_key<T> >& key_set)
    { this->interface_ref().erase(key_set); }

    void clear()
    { this->interface_ref().clear(); }
};

/******************************************************************************/

template <typename T>
struct poly_sequence_view
{
    typedef poly< sequence_view<T> > type;
};

/******************************************************************************/

} // namespace adobe

/******************************************************************************/

// ADOBE_POLY_SEQUENCE_VIEW_HPP
#endif

/******************************************************************************/
