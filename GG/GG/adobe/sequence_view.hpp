/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/******************************************************************************/

#ifndef ADOBE_SEQUENCE_VIEW_HPP
#define ADOBE_SEQUENCE_VIEW_HPP

/******************************************************************************/

#include <boost/concept_check.hpp>
#include <boost/function.hpp>

#include <GG/adobe/copy_on_write.hpp>
#include <GG/adobe/selection.hpp>
#include <GG/adobe/vector.hpp>

/******************************************************************************/

namespace adobe {

/******************************************************************************/
/*!
    \defgroup sequence_view Sequence View Concept
    \ingroup apl_libraries

    \brief SequenceView concept for sequences.
*/
/******************************************************************************/
/*!
    \ingroup sequence_view

    \brief SequenceView concept requirement.
*/
template <class SequenceView>
struct sequence_view_key_type
{
    /// type for sequence_view_key_type
    typedef typename SequenceView::key_type type;
};

template <class SequenceView>
struct sequence_view_value_type
{
    /// type for sequence_view_value_type
    typedef typename SequenceView::value_type type;
};

template <class SequenceView>
struct sequence_view_cow_value_type
{
    /// type for sequence_view_cow_value_type
    typedef typename SequenceView::cow_value_type type;
};

/******************************************************************************/

/*!
    \ingroup sequence_view

    \brief SequenceView concept requirement.
*/
template <class SV> // SV models SequenceView
inline void refresh(SV&                                             v,
                    typename sequence_view_key_type<SV>::type       index,
                    typename sequence_view_cow_value_type<SV>::type value)
{ v.refresh(index, value); }

/*!
    \ingroup sequence_view

    \brief SequenceView concept requirement.
*/
template <class SV> // SV models SequenceView
inline void extend(SV&                                             v,
                   typename sequence_view_key_type<SV>::type       before,
                   typename sequence_view_key_type<SV>::type       value_key,
                   typename sequence_view_cow_value_type<SV>::type value)
{ v.extend(before, value_key, value); }

/*!
    \ingroup sequence_view

    \brief SequenceView concept requirement.
*/
template <class SV> // SV models SequenceView
inline void extend_set(SV&                                                  v,
                   typename sequence_view_key_type<SV>::type                before,
                   const vector<typename sequence_view_key_type<SV>::type>& extend_key_set)
{ v.extend_set(before, extend_key_set); }

/*!
    \ingroup sequence_view

    \brief SequenceView concept requirement.
*/
template <class SV> // SV models SequenceView
inline void erase(SV&                                                      v,
                  const vector<typename sequence_view_key_type<SV>::type>& key_set)
{ v.erase(key_set); }

/*!
    \ingroup sequence_view

    \brief SequenceView concept requirement.
*/
template <class SV> // SV models SequenceView
inline void clear(SV& v)
{ v.clear(); }

/******************************************************************************/
/*!
    \ingroup sequence_view

    \brief View concept for sequences.
*/
template <class SequenceView>
struct SequenceViewConcept
{
    /// key_type requirement for the SequenceViewConcept
    typedef typename sequence_view_key_type<SequenceView>::type       key_type; 
    typedef typename sequence_view_cow_value_type<SequenceView>::type cow_value_type; 

    /// functional constraints for a model of the SequenceViewConcept
    void constraints()
    {
        refresh(*view, index, value);
        extend(*view, index, index, value);
        extend_set(*view, index, key_set);
        erase(*view, key_set);
    }

    /// refreshes the SequenceView at a given position in the sequence
    static void refresh(SequenceView&  view,
                        key_type       index,
                        cow_value_type value)
    {
        using adobe::refresh;

        refresh(view, index, value);
    }

    /// notifes the SequenceView of the insertion of an element into the sequence
    static void extend(SequenceView&  view,
                       key_type       before,
                       key_type       value_key,
                       cow_value_type value)
    {
        using adobe::extend;

        (extend)(view, before, value_key, value);
    }

    /// notifes the SequenceView of the insertion of elements into the sequence
    static void extend_set(SequenceView&           view,
                           key_type                before,
                           const vector<key_type>& key_set)
    {
        using adobe::extend_set;

        extend_set(view, before, key_set);
    }

    /// notifes the SequenceView of the elimination of elements from the sequence
    static void erase(SequenceView&           view,
                      const vector<key_type>& key_set)
    {
        using adobe::erase;

        erase(view, key_set);
    }

    /// notifes the SequenceView of the elimination of all elements from the sequence
    static void clear(SequenceView& view)
    {
        using adobe::clear;

        clear(view);
    }

#ifndef ADOBE_NO_DOCUMENTATION
    SequenceView*    view;
    key_type         index;
    cow_value_type   value;
    vector<key_type> key_set;
#endif
};

/******************************************************************************/
/*!
    \ingroup sequence_view
    a concept-map type that permits use of boost::reference_wrapper with SequenceViewConcept
*/
template <class T>
struct SequenceViewConcept<boost::reference_wrapper<T> > : SequenceViewConcept<T> 
{
    void constraints() {
        //boost concept check lib gets confused on VC8 without this
        SequenceViewConcept<T>::constraints();
    }
};

/******************************************************************************/

} // namespace adobe

/******************************************************************************/

// ADOBE_SEQUENCE_VIEW_HPP
#endif

/******************************************************************************/
