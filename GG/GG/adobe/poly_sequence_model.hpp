/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/******************************************************************************/

#ifndef ADOBE_POLY_SEQUENCE_MODEL_HPP
#define ADOBE_POLY_SEQUENCE_MODEL_HPP

/******************************************************************************/

#include <boost/concept_check.hpp>

#include <GG/adobe/sequence_model_fwd.hpp>
#include <GG/adobe/poly.hpp>
#include <GG/adobe/vector.hpp>

/******************************************************************************/

namespace adobe {

/******************************************************************************/
/*!
    \defgroup sequence_mvc Sequence MVC Concepts
        \ingroup apl_libraries

        \defgroup sequence_model_concept Sequence Model Concept
    \ingroup sequence_mvc

    \brief SequenceModel concept for sequences.
*/
/******************************************************************************/
/*!
    \ingroup sequence_model_concept

    \brief SequenceModel concept requirement.
*/
template <class SequenceModel>
struct sequence_model_value_type
{
    typedef typename SequenceModel::value_type type;
};

/*!
    \ingroup sequence_model_concept

    \brief SequenceModel concept requirement.
*/
template <class SequenceModel>
struct sequence_model_key_type
{
    typedef typename SequenceModel::key_type type;
};

/******************************************************************************/
/*!
    \ingroup sequence_model_concept

    \brief SequenceModel concept requirement.
*/
template <class SM> // SM models SequenceModel
inline void push_back(SM&                                                 v,
                      const typename sequence_model_value_type<SM>::type& x)
{ v.push_back(x); }

/*!
    \ingroup sequence_model_concept

    \brief SequenceModel concept requirement.
*/
template <class SM> // SM models SequenceModel
inline void set(SM&                                                 v,
                typename sequence_model_key_type<SM>::type          key,
                const typename sequence_model_value_type<SM>::type& x)
{ v.set(key, x); }

/*!
    \ingroup sequence_model_concept

    \brief SequenceModel concept requirement.
*/
template <class SM> // SM models SequenceModel
inline void insert_set(SM&                                                         v,
                       typename sequence_model_key_type<SM>::type                  before,
                       const vector<typename sequence_model_value_type<SM>::type>& x)
{ v.insert_set(before, x); }

/*!
    \ingroup sequence_model_concept

    \brief SequenceModel concept requirement.
*/
template <class SM> // SM models SequenceModel
inline void insert(SM&                                                 v,
                   typename sequence_model_key_type<SM>::type          before,
                   const typename sequence_model_value_type<SM>::type& x)
{ v.insert(before, x); }

/*!
    \ingroup sequence_model_concept

    \brief SequenceModel concept requirement.
*/
template <class SM> // SM models SequenceModel
inline void sequence_model_erase(SM&                                                       v,
                                 const vector<typename sequence_model_key_type<SM>::type>& x)
{ v.erase(x); }

/*!
    \ingroup sequence_model_concept

    \brief SequenceModel concept requirement.
*/
template <class SM> // SM models SequenceModel
inline void sequence_model_clear(SM& v)
{ v.clear(); }

/******************************************************************************/
/*!
    \ingroup sequence_model_concept

    \brief SequenceModel concept requirement.
*/
template <class SequenceModel>
struct SequenceModelConcept
{
    /// value_type requirement for the SequenceModelConcept
    typedef typename sequence_model_value_type<SequenceModel>::type value_type; 
    typedef typename sequence_model_key_type<SequenceModel>::type   key_type; 

    /// functional constraints for a model of the SequenceModelConcept
    void constraints()
    {
        push_back(*model, *value);
        set(*model, *key, *value);
        insert_set(*model, *key, *value_set);
        insert(*model, *key, *value);
        sequence_model_erase(*model, *key_set);
        sequence_model_clear(*model);
    }

    static void push_back(SequenceModel& model, const value_type& x)
    {
        using adobe::push_back;

        push_back(model, x);
    }

    static void set(SequenceModel& model, key_type key, const value_type& x)
    {
        using adobe::set;

        set(model, key, x);
    }

    static void insert_set(SequenceModel& model, key_type before, const vector<value_type>& x)
    {
        using adobe::insert_set;

        insert_set(model, before, x);
    }

    static void insert(SequenceModel& model, key_type before, const value_type& x)
    {
        using adobe::insert;

        insert(model, before, x);
    }

    static void erase(SequenceModel& model, const vector<key_type>& x)
    {
        using adobe::sequence_model_erase;

        sequence_model_erase(model, x);
    }

    static void clear(SequenceModel& model)
    {
        using adobe::sequence_model_clear;

        sequence_model_clear(model);
    }

#ifndef ADOBE_NO_DOCUMENTATION
    SequenceModel*      model;
    key_type*           key;
    value_type*         value;
    vector<value_type>* value_set;
    vector<key_type>*   key_set;
#endif
};

/******************************************************************************/
/*!
    \ingroup sequence_model_concept
    a concept-map type that permits use of boost::reference_wrapper with SequenceModelConcept
*/
template <class T>
struct SequenceModelConcept<boost::reference_wrapper<T> > : SequenceModelConcept<T> 
{
    void constraints() {
        //boost concept check lib gets confused on VC8 without this
        SequenceModelConcept<T>::constraints();
    }
};

/******************************************************************************/
#if 0
#pragma mark -
#endif
/******************************************************************************/
/*!
    \ingroup sequence_mvc

    \brief poly_ concept for implementations that model the SequenceModel concept.
*/
template <typename T>
struct poly_sequence_model_interface : poly_copyable_interface
{
    virtual void push_back(const T& x) = 0;

    virtual void set(sequence_key<T> key, const T& value) = 0;

    virtual void insert_set(sequence_key<T> before, const vector<T>& value_set) = 0;

    virtual void insert(sequence_key<T> before, const T& value) = 0;

    virtual void erase(const vector<sequence_key<T> >& key_set) = 0;

    virtual void clear() = 0;
};

/******************************************************************************/
/*!
    \ingroup sequence_mvc

    \brief poly_ instance for implementations that model the SequenceModel concept.
*/
template <typename T>
struct poly_sequence_model_instance
{
    template <typename V>
    struct type : optimized_storage_type<V, poly_sequence_model_interface<T> >::type
    {
        typedef typename optimized_storage_type<V, poly_sequence_model_interface<T> >::type base_t;

        BOOST_CLASS_REQUIRE(V, adobe, SequenceModelConcept);

        explicit type(const V& x) :
            base_t(x)
        { }

        type(move_from<type> x) :
            base_t(move_from<base_t>(x.source))
        { }

        void push_back(const T& x)
        { SequenceModelConcept<V>::push_back(this->get(), x); }
    
        void set(sequence_key<T> key, const T& x)
        { SequenceModelConcept<V>::set(this->get(), key, x); }

        void insert_set(sequence_key<T> before, const vector<T>& value_set)
        { SequenceModelConcept<V>::insert_set(this->get(), before, value_set); }

        void insert(sequence_key<T> before, const T& x)
        { SequenceModelConcept<V>::insert(this->get(), before, x); }

        void erase(const vector<sequence_key<T> >& key_set)
        { SequenceModelConcept<V>::erase(this->get(), key_set); }

        void clear()
        { SequenceModelConcept<V>::clear(this->get()); }
    };
};

/******************************************************************************/
/*!
    \ingroup sequence_mvc

    \brief poly_ holder for implementations that model the SequenceModel concept.
*/
template <typename T>
struct sequence_model_base : poly_base<poly_sequence_model_interface<T>,
                                       poly_sequence_model_instance<T>::template type>
{
    typedef poly_base<poly_sequence_model_interface<T>,
                      poly_sequence_model_instance<T>::template type> base_t;

    template <typename V>
    explicit sequence_model_base(const V& s) :
        base_t(s)
    { }

    sequence_model_base(move_from<sequence_model_base> x) :
        base_t(move_from<base_t>(x.source))
    { }

    void push_back(const T& x)
    { this->interface_ref().push_back(x); }

    void set(sequence_key<T> key, const T& x)
    { this->interface_ref().set(key, x); }

    void insert_set(sequence_key<T> before, const vector<T>& value_set)
    { this->interface_ref().insert_set(before, value_set); }

    void insert(sequence_key<T> before, const T& x)
    { this->interface_ref().insert(before, x); }

    void erase(const vector<sequence_key<T> >& key_set)
    { this->interface_ref().erase(key_set); }

    void clear()
    { this->interface_ref().clear(); }
};

/******************************************************************************/

template <typename T>
struct poly_sequence_model
{
    typedef poly< sequence_model_base<T> > type;
};

/******************************************************************************/

} // namespace adobe

/******************************************************************************/

// ADOBE_POLY_SEQUENCE_MODEL_HPP
#endif

/******************************************************************************/
