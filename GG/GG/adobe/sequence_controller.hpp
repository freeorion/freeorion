/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/******************************************************************************/

#ifndef ADOBE_SEQUENCE_CONTROLLER_HPP
#define ADOBE_SEQUENCE_CONTROLLER_HPP

/******************************************************************************/

#include <boost/concept_check.hpp>
#include <boost/function.hpp>

#include <GG/adobe/poly_sequence_model.hpp>
#include <GG/adobe/vector.hpp>

/******************************************************************************/

namespace adobe {

/******************************************************************************/
/*!
    \defgroup sequence_controller Sequence Controller Concept
    \ingroup apl_libraries

    \brief SequenceController concept for sequences.
*/
/******************************************************************************/
/*!
    \ingroup sequence_controller

    \brief SequenceController concept requirement.
*/
template <class SequenceController>
struct sequence_controller_value_type
{
    /// type for sequence_controller_value_type
    typedef typename SequenceController::value_type type;
};

/******************************************************************************/
/*!
    \ingroup sequence_controller

    \brief SequenceController concept requirement.
*/
template <class SC> // SC models SequenceController
inline void monitor_sequence(SC&                                                                           v,
                             typename poly_sequence_model<typename sequence_controller_value_type<SC>::type>::type& sequence)
{ v.monitor_sequence(sequence); }

/******************************************************************************/
/*!
    \ingroup sequence_controller

    \brief SequenceController concept requirement.
*/
template <class SequenceController>
struct SequenceControllerConcept
{
    /// value_type requirement for the SequenceControllerConcept
    typedef typename sequence_controller_value_type<SequenceController>::type value_type; 

    /// functional constraints for a model of the SequenceControllerConcept
    void constraints()
    {
        monitor_sequence(*controller, *sequence);
    }

    /// pushes to the controller a sequence to monitor
    static void monitor_sequence(SequenceController& controller, typename poly_sequence_model<value_type>::type& sequence)
    {
        using adobe::monitor_sequence;

        monitor_sequence(controller, sequence);
    }

#ifndef ADOBE_NO_DOCUMENTATION
    SequenceController*                    controller;
    typename poly_sequence_model<value_type>::type* sequence;
#endif
};

/******************************************************************************/
/*!
    \ingroup sequence_controller
    a concept-map type that permits use of boost::reference_wrapper with SequenceControllerConcept
*/
template <class T>
struct SequenceControllerConcept<boost::reference_wrapper<T> > : SequenceControllerConcept<T> 
{
    void constraints() {
        //boost concept check lib gets confused on VC8 without this
        SequenceControllerConcept<T>::constraints();
    }
};

/******************************************************************************/

} // namespace adobe

/******************************************************************************/

// ADOBE_SEQUENCE_CONTROLLER_HPP
#endif

/******************************************************************************/
