/*
    Copyright 2006-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/
#ifndef ADOBE_POLY_PLACEABLE_HPP
#define ADOBE_POLY_PLACEABLE_HPP

#include <boost/ref.hpp>

#include <GG/adobe/config.hpp>
#include <GG/adobe/poly.hpp>
#include <GG/adobe/placeable_concept.hpp>
#include <GG/adobe/layout_attributes.hpp>



/*************************************************************************************************/

namespace adobe {
    
/*************************************************************************************************/
/*!
\defgroup poly_placeable poly_placeable
\ingroup layout_library
 */
/*!

\brief Pure virtual interface for the poly<placeable> modeling
\ref adobe::PlaceableConcept
\ingroup poly_placeable
*/
    
struct poly_placeable_interface : poly_copyable_interface
{
    virtual void measure(extents_t& result) = 0;
    virtual void place(const place_data_t& place_data) = 0;
};

/*************************************************************************************************/

/*!

\brief Implementation of virtual interface for the poly<placeable> modeling
\ref adobe::PlaceableConcept 
\ingroup poly_placeable
*/

template <typename T>
struct poly_placeable_instance : optimized_storage_type<T, poly_placeable_interface>::type
{
    typedef typename optimized_storage_type<T, poly_placeable_interface>::type base_t;

    BOOST_CLASS_REQUIRE(T, adobe, PlaceableConcept);

    poly_placeable_instance(const T& x) 
        : base_t(x) {}
    poly_placeable_instance(move_from<poly_placeable_instance> x) 
        : base_t(move_from<base_t>(x.source)) {}

    void measure(extents_t& result)
    { 
        PlaceableConcept<T>::measure(this->get(), result); 
    }

    void place(const place_data_t& place_data)
    { 
        PlaceableConcept<T>::place(this->get(), place_data); 
    }
};

    
/*************************************************************************************************/

/*!

\brief Representative of \ref adobe::PlaceableConcept so that placeable
models \ref adobe::PlaceableConcept when T does.
\ingroup poly_placeable
*/

struct placeable : public poly_base<poly_placeable_interface, poly_placeable_instance>
{
    typedef poly_base<poly_placeable_interface, poly_placeable_instance> base_t;

    template <typename T>
    explicit placeable(const T& x) : base_t(x) {}
    
    placeable(move_from<placeable> x) : base_t(move_from<base_t>(x.source)) {}

    void measure(extents_t& result)
    { interface_ref().measure(result); }

    void place(const place_data_t& place_data)
    { interface_ref().place(place_data); }
};

/*************************************************************************************************/

/*!

\ingroup poly_placeable

\brief convenience typedef. \sa adobe::PlaceableConcept .


*/
typedef poly<placeable> poly_placeable_t;

/*************************************************************************************************/

/*!

\brief Pure virtual interface for poly<placeable_twopass> modeling
\ref adobe::PlaceableTwoPassConcept
\ingroup poly_placeable
*/
struct poly_placeable_twopass_interface : public poly_placeable_interface
{
    virtual void measure_vertical(extents_t& in_out_attrs,
                                  const place_data_t& placed_horizontal) = 0;
};

/*************************************************************************************************/

/*!
\brief Implementation of virtual interface for the poly<placeable_twopass> modeling
\ref adobe::PlaceableTwoPassConcept 
\ingroup poly_placeable
*/
template <typename T>
struct poly_placeable_twopass_instance : optimized_storage_type<T, poly_placeable_twopass_interface>::type
{
    typedef typename optimized_storage_type<T, poly_placeable_twopass_interface>::type base_t;

    BOOST_CLASS_REQUIRE(T, adobe, PlaceableTwoPassConcept);

    poly_placeable_twopass_instance(const T& x) 
        : base_t(x) {}  
    poly_placeable_twopass_instance(move_from<poly_placeable_twopass_instance> x) 
        : base_t(move_from<base_t>(x.source)) {}

    void measure(extents_t& result)
    { 
        PlaceableTwoPassConcept<T>::measure(this->get(), result); 
    }
    
    void measure_vertical(extents_t& calculated_horizontal, const place_data_t& placed_horizontal)
    { 
        PlaceableTwoPassConcept<T>::measure_vertical(this->get(), calculated_horizontal, placed_horizontal); 
    }

    void place(const place_data_t& place_data)
    { 
        PlaceableTwoPassConcept<T>::place(this->get(), place_data); 
    }
    
};

/*************************************************************************************************/

/*!
\ingroup poly_placeable
*/
struct placeable_twopass 
    : public poly_base<poly_placeable_twopass_interface, poly_placeable_twopass_instance>
{
    typedef poly_base<poly_placeable_twopass_interface, poly_placeable_twopass_instance> base_t;

    template <typename T>
    explicit placeable_twopass(const T& x) : base_t(x) {}
 
    placeable_twopass(move_from<placeable_twopass> x) : base_t(move_from<base_t>(x.source)) {}

    void measure(extents_t& result)
        { interface_ref().measure(result); }

    void measure_vertical(extents_t& calculated_horizontal, const place_data_t& placed_horizontal)
        { interface_ref().measure_vertical(calculated_horizontal, placed_horizontal); }

    void place(const place_data_t& place_data)
        { interface_ref().place(place_data); }
};

/*************************************************************************************************/

/*!

\ingroup poly_placeable

\brief convenience typedef. \sa adobe::PlaceableTwoPassConcept .


*/

typedef poly<placeable_twopass> poly_placeable_twopass_t;

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/

#endif
