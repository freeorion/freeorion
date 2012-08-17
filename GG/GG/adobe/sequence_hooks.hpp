/*
        Copyright 2008 Adobe Systems Incorporated
        Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
        or a copy at http://stlab.adobe.com/licenses.html)
*/

/******************************************************************************/

#ifndef ADOBE_SEQUENCE_HOOKS
#define ADOBE_SEQUENCE_HOOKS

/******************************************************************************/

#include <GG/adobe/sequence_model.hpp>
#include <GG/adobe/future/assemblage.hpp>

/******************************************************************************/

namespace adobe {

/******************************************************************************/
/*!
    Hooks an object that models the SequenceView concept to a sequence_model
*/
template <typename SequenceModel, typename SequenceView>
void attach_sequence_view_to_sequence_model(adobe::assemblage_t& assemblage,
                                            SequenceModel&       model,
                                            SequenceView&        view)
{
    typedef typename SequenceModel::poly_sequence_view_type sequence_view_type;

    sequence_view_type* poly_view(new sequence_view_type(boost::ref(view)));

    assemblage_cleanup_ptr(assemblage, poly_view);

    model.attach_view(*poly_view);

    assemblage.cleanup(boost::bind(&SequenceModel::detach_view,
                                   boost::ref(model), 
                                   boost::ref(*poly_view)));
}

/******************************************************************************/
/*!
    Hooks an object that models the SequenceView concept to a sequence_model
*/
template <typename SequenceModel, typename SequenceController>
void attach_sequence_controller_to_sequence_model(adobe::assemblage_t& assemblage,
                                                  SequenceModel&       model,
                                                  SequenceController&  controller)
{
    typedef typename SequenceModel::poly_sequence_controller_type sequence_controller_type;

    sequence_controller_type* poly_controller(new sequence_controller_type(boost::ref(controller)));

    assemblage_cleanup_ptr(assemblage, poly_controller);

    model.attach_controller(*poly_controller);

    assemblage.cleanup(boost::bind(&SequenceModel::detach_controller,
                                   boost::ref(model), 
                                   boost::ref(*poly_controller)));
}

/******************************************************************************/
/*!
    Hooks an object that models the SequenceView concept to a sequence_model
*/
template <typename SequenceModel, typename SequenceWidget>
inline void attach_sequence_widget_to_sequence_model(adobe::assemblage_t& assemblage,
                                                     SequenceModel&       model,
                                                     SequenceWidget&      widget)
{
    attach_sequence_view_to_sequence_model(assemblage, model, widget);

    attach_sequence_controller_to_sequence_model(assemblage, model, widget);
}

/******************************************************************************/

} // namespace adobe

/******************************************************************************/
// ADOBE_SEQUENCE_HOOKS
#endif
/******************************************************************************/
