/*
    Copyright 2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/**************************************************************************************************/

#ifndef ADOBE_DRAG_AND_DROP_CONVERTER_CONCEPT_HPP
#define ADOBE_DRAG_AND_DROP_CONVERTER_CONCEPT_HPP

/**************************************************************************************************/

#include <boost/concept_check.hpp>
#include <boost/ref.hpp>

#include <GG/adobe/regular_concept.hpp>

/**************************************************************************************************/

namespace adobe {

/*************************************************************************************************/
/*!
    \defgroup drag_and_drop_converter Drag and Drop Converter Concept
        \ingroup apl_libraries

    \brief DragAndDropConverter concept requirement.
*/

/*************************************************************************************************/
    
/*!
    \ingroup drag_and_drop_converter
*/
template <class Converter>
struct dnd_converter_source_type 
{
    typedef typename boost::unwrap_reference<Converter>::type::source_type type;
};

/*************************************************************************************************/
    
/*!
    \ingroup drag_and_drop_converter
*/
template <class Converter>
struct dnd_converter_dest_type 
{
    typedef typename boost::unwrap_reference<Converter>::type::dest_type type;
};

/*************************************************************************************************/
/*!
    \ingroup drag_and_drop_converter
*/
template <class DNDC> // DNDC models DragAndDropConverter
inline typename dnd_converter_dest_type<DNDC>::type
convert(const DNDC&                                           dndc,
        const typename dnd_converter_source_type<DNDC>::type& raw_value)
{ return dndc.convert(raw_value); }

/*************************************************************************************************/
/*!
    \ingroup drag_and_drop_converter

    \brief DragAndDropConverter concept requirement.
*/
template <class DragAndDropConverter>
struct DragAndDropConverterConcept
{
    /// source_type requirement for the DragAndDropConverterConcept
    typedef typename dnd_converter_source_type<DragAndDropConverter>::type source_type; 
    typedef typename dnd_converter_dest_type<DragAndDropConverter>::type   dest_type; 

    /// notifies the converter whether or not it should
    /// be enabled (and thus able to modify the model)
    static dest_type convert(const DragAndDropConverter& converter, const source_type& src_value)
    {
        using adobe::convert;
        return convert(converter, src_value);
    }

#ifndef ADOBE_NO_DOCUMENTATION
    // Concept checking:
    // Use pointers since not required to be default constructible

    const DragAndDropConverter* converter;
    source_type*                src_value;
    dest_type*                  dest_value;
#endif

    /// functional constraints for a model of the DragAndDropConverterConcept
    void constraints() {
        // associated types:
        typedef typename dnd_converter_source_type<DragAndDropConverter>::type associated_type1;
        typedef typename dnd_converter_dest_type<DragAndDropConverter>::type associated_type2;

        using adobe::convert;
        convert(*converter, *src_value);
    }
};

/*!
    \ingroup drag_and_drop_converter
    a concept-map type that permits use of boost::reference_wrapper with SequenceViewConcept
*/
template <class DragAndDropConverter>
struct DragAndDropConverterConcept<boost::reference_wrapper<DragAndDropConverter> > : 
    DragAndDropConverterConcept<DragAndDropConverter>
{
    void constraints() {
        //boost concept check lib gets confused on VC8 without this
        DragAndDropConverterConcept<DragAndDropConverter>::constraints();
    }
};

/*************************************************************************************************/

} // namespace adobe

/**************************************************************************************************/

// ADOBE_DRAG_AND_DROP_CONVERTER_CONCEPT_HPP
#endif

/**************************************************************************************************/
