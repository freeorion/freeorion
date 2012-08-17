/*
    Copyright 2006-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/
#ifndef ADOBE_PLACEABLE_HPP
#define ADOBE_PLACEABLE_HPP

/************************************************************************************************/

#include <boost/concept_check.hpp>

#include <GG/adobe/extents.hpp>
#include <GG/adobe/layout_attributes.hpp>

/************************************************************************************************/

namespace adobe {
/*!
\defgroup placeable_concept Placeable Concept
\ingroup layout_library
*/


/************************************************************************************************/

#ifdef ADOBE_HAS_CPLUS0X_CONCEPTS

/*************************************************************************************************/


/*!
\ingroup placeable_concept
 */
auto concept PlaceableConcept <typename T> 
// not yet : RegularConcept<T> 
: std::CopyConstructible<T>
{
    void measure(T& t, extents_t& result);
    void place(T& t, const place_data_t& place_data);
};

/*************************************************************************************************/

/*!
\ingroup placeable_concept
 */
auto concept PlaceableMFConcept <typename Placeable> 
// not yet : RegularConcept<Placeable>
: std::CopyConstructible<Placeable>
{
    void Placeable::measure(extents_t& result); 
    void Placeable::place(const place_data_t& place_data);
};

/*************************************************************************************************/

/*!
\ingroup placeable_concept
 */
template <PlaceableMFConcept T>
concept_map PlaceableConcept<T> {
    void measure(T& t, extents_t& result)
        { t.measure(result); }
    void place(T& t, const place_data_t& place_data)
        { t.place(place_data); }
};

/*************************************************************************************************/

/*!
\ingroup placeable_concept
 */
template <PlaceableConcept T>
concept_map PlaceableConcept<boost::reference_wrapper<T> > {
    void measure(boost::reference_wrapper<T>& r, extents_t& result)
        { PlaceableConcept<T>::measure(static_cast<T&>(r), result); }
};

/*************************************************************************************************/

/*!
\ingroup placeable_concept
 */
auto concept PlaceableTwoPassConcept<typename Placeable> : PlaceableConcept<Placeable> 
{
    void measure_vertical(Placeable& p, extents_t& calculated_horizontal,
                                 const place_data_t& placed_horizontal);
};

/*************************************************************************************************/

/*!
\ingroup placeable_concept
 */
auto concept PlaceableTwoPassMFConcept<typename Placeable> : PlaceableMFConcept<Placeable>
{
    void Placeable::measure_vertical(extents_t& calculated_horizontal,
                                 const place_data_t& placed_horizontal); 
};

/*************************************************************************************************/

/*!
\ingroup placeable_concept
 */
template <typename T>
concept_map PlaceableTwoPassConcept<PlaceableTwoPassMFConcept<T> > {
    
    void measure(PlaceableTwoPassMFConcept<T>& t, extents_t& result)
    { t.measure(result); }

    void place(PlaceableTwoPassMFConcept<T>& t, const place_data_t& place_data)
    { t.place(place_data); }

    void measure_vertical(PlaceableTwoPassMFConcept<T>& t, extents_t& calculated_horizontal,
                                 const place_data_t& placed_horizontal)
    { t.measure_vertical(calculated_horizontal, placed_horizontal); }
};

/*************************************************************************************************/

/*!
\ingroup placeable_concept
 */
template <typename T>
concept_map PlaceableTwoPassConcept<boost::reference_wrapper<PlaceableTwoPassConcept<T> > > {
    void measure(boost::reference_wrapper<PlaceableTwoPassConcept<T> >& r, 
                          extents_t& extents)
    { 
        PlaceableTwoPassConcept<PlaceableTwoPassConcept<T> >::measure(
            *r.get_pointer(), extents); 
    }
    void place(boost::reference_wrapper<PlaceableTwoPassConcept<T> >& r, 
               const place_data_t& place_data)
    { 
        PlaceableTwoPassConcept<PlaceableTwoPassConcept<T> >::place(
            *r.get_pointer(), place_data); 
    }
    void measure_vertical(boost::reference_wrapper<PlaceableTwoPassConcept<T> >& r, 
                          extents_t& calculated_horizontal,
                          const place_data_t& placed_horizontal)
    { 
        PlaceableTwoPassConcept<PlaceableTwoPassConcept<T> >::measure_vertical(
            *r.get_pointer(), calculated_horizontal, placed_horizontal); 
    }
};

/*************************************************************************************************/

#else

/*************************************************************************************************/


/*!

\ingroup placeable_concept

Get the placeable object-specific information from the client. The
client is allowed to modify the \ref extents_t parameter passed in as
they please in order to best describe the geometric details of this
placeable object... It is an ouput-only parameter. This is the second
(first if the placeable object is not a container) proc called while
the layout engine is solving the layout.

The default implementation calls the member function on T of the same
name. Can be specialized or overloaded for user types.

*/

template <class T>
inline void measure(T& t, extents_t& result)
{ t.measure(result); }

/*************************************************************************************************/


/*!

\ingroup placeable_concept

Place is the final function signaled from the Eve engine to a placeable object. When this function 
is made there are several guarantees for the client:
    - All parent views for this placeable object have already had their place function invoked
    - All data provided in the place_data_t is the final geometric information for the placeable 
    object.

\par

    Note that this function can be invoked multiple times- given a
    view re-layout (i.e. during a window resize event) the other
    functions will not be invoked- the layout will be adjusted
    internally in the Eve engine and only this function will be
    invoked with updated placement information for the placeable
    object.

The default implementation calls the member function on T of the same
name. Can be specialized or overloaded for user types.

*/

template <class T>
inline void place(T& t, const place_data_t& place_data)
{ t.place(place_data); }

/*************************************************************************************************/

/*!

\ingroup placeable_concept

\brief Concept map and constraints checking for the Placeable concept

*/

template <class T>
struct PlaceableConcept
{
#if !defined(ADOBE_NO_DOCUMENTATION)

    static void measure(T& t, extents_t& result)
    {   
        using adobe::measure; // pick up default version which looks for member functions
        measure(t, result); // unqualified to allow user versions
    }

    static void place(T& t, const place_data_t& place_data)
    { 
        using adobe::place; // pick up default version which looks for member functions
        place(t, place_data); // unqualified to allow user versions
    }

// Concept checking:
    
    void constraints() {
        // not yet: boost::function_requires<RegularConcept<Placeable> >();
        // boost::function_requires<boost::CopyConstructibleConcept<Placeable> >();

        using adobe::measure; 
        measure(*placeable, extents);

        using adobe::place;
        place(*placeable, place_data);
    }

//use pointers since not required to be default constructible
    T*                  placeable;
    const place_data_t  place_data;
    extents_t           extents;    
#endif
};

/*!

\brief Concept map and constraints checking for to allow
boost::reference_wrapper<T> to model this concept when T does.
\ingroup placeable_concept

*/

template <class T>
struct PlaceableConcept<T*> : public PlaceableConcept<T>
{
    static void measure(T* r, extents_t& result) 
    { PlaceableConcept<T>::measure(*r, result); }

    static void place(T* r, const place_data_t& place_data) 
    { PlaceableConcept<T>::place(*r, place_data); }

#if !defined(ADOBE_NO_DOCUMENTATION)
    void constraints() {
        //boost concept check lib gets confused on VC8 without this
        PlaceableConcept<T>::constraints();
    }
#endif
};

/*************************************************************************************************/


/*!

\ingroup placeable_concept

Sometimes a placeable object's height is dependent upon the solved
width of that placeable object. An example of this would be a
placeable object with paginated text inside of it. In that case, the
placeable object should provide a function here to remeasure <i>just
the vertical slice</i> information for the placeable object. All other
information modified here will be discarded in favor of the values
given during the first calculate call. Calculated_vertical is an
output-only parameter. Horizontal and positioning information should
be obtained from placed_horizontal.

The default implementation calls the member function on T of the same
name. Can be specialized or overloaded for user types.

*/

template <class T>
inline void measure_vertical(T& t,
                             extents_t& calculated_horizontal,
                             const place_data_t& placed_horizontal)
{ t.measure_vertical(calculated_horizontal, placed_horizontal); }

/*************************************************************************************************/

/*!

\ingroup placeable_concept

\brief Concept map and constraints checking for the PlaceableTwoPass concept

*/

template <class T>
struct PlaceableTwoPassConcept : PlaceableConcept<T>
{
#if ! defined(ADOBE_NO_DOCUMENTATION)    

    static void measure_vertical(T& t,
                                 extents_t& calculated_horizontal,
                                 const place_data_t& placed_horizontal)
    {
        using adobe::measure_vertical;
        measure_vertical(t, calculated_horizontal, placed_horizontal);
    }

    void constraints() {
        // not yet: boost::function_requires<RegularConcept<T> >();
        //boost::function_requires<boost::CopyConstructibleConcept<T> >();  

        using adobe::place;
        place(*t2, this->place_data); 
        
        using adobe::measure;
        measure(*t2, this->extents);

        using adobe::measure_vertical;
        measure_vertical(*t2, this->extents, this->place_data);
    }

     // Concept checking:
    //use pointers since not required to be default constructible
    T *t2;
#endif

};

/*!

\brief Concept map and constraints checking for to allow
boost::reference_wrapper<T> to model this concept when T does.
\ingroup placeable_concept

*/

template <class T>
struct PlaceableTwoPassConcept<T*> : 
    PlaceableTwoPassConcept<T> 
{
    static void measure(T* r, extents_t& extents)
    { 
        PlaceableTwoPassConcept<T>::measure(
            *r, extents); 
    }
    static void place(T* r, const place_data_t& place_data)
    { 
        PlaceableTwoPassConcept<T>::place(
            *r, place_data); 
    }
    static void measure_vertical(T* r, extents_t& calculated_horizontal,
                          const place_data_t& placed_horizontal)
    { 
        PlaceableTwoPassConcept<T>::measure_vertical(
            *r, calculated_horizontal, placed_horizontal); 
    }

#if !defined(ADOBE_NO_DOCUMENTATION)
    void constraints() {
        //boost concept check lib gets confused on VC8 without this
        PlaceableTwoPassConcept<T>::constraints();
    }
#endif
};


/*************************************************************************************************/

#endif

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/

#endif
