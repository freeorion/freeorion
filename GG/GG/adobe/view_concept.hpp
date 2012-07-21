/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_VIEW_HPP
#define ADOBE_VIEW_HPP

/*************************************************************************************************/

#include <boost/concept_check.hpp>
#include <boost/ref.hpp>

#include <GG/adobe/regular_concept.hpp>

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/

#ifdef ADOBE_HAS_CPLUS0X_CONCEPTS

/*************************************************************************************************/

auto concept ViewConcept<typename View> 
//: RegularConcept<View> -- Views not yet regular
: std::CopyConstructible<View>
{
    typename model_type;
    void display(View& v, model_type value); 
};

/*************************************************************************************************/

auto concept ViewMFConcept<typename View> 
//: RegularConcept<View> -- Views not yet regular
: std::CopyConstructible<View>
{
        
    typename model_type = View::model_type;
    void View::display(model_type value); 
};

/*************************************************************************************************/

template <ViewMFConcept T>
concept_map ViewConcept<T> {
    typedef ViewMFConcept<T>::model_type model_type;
    inline void display(T& v, model_type value)
    { v.display(value); }
};

/*************************************************************************************************/

template <ViewConcept T>
concept_map ViewConcept<boost::reference_wrapper<T> > {
    typedef ViewConcept<T>::model_type model_type;
    void display(boost::reference_wrapper<T>& r, model_type value)
    { ViewConcept<T>::display(static_cast<T&>(r),value); }
};

/*************************************************************************************************/

#else

/*************************************************************************************************/
    
template <class View>
struct view_model_type 
{
    typedef typename boost::unwrap_reference<View>::type::model_type type;
};

/*************************************************************************************************/

template <class V> // V models View
inline void display(V& v, const typename view_model_type<V>::type& value)
{ v.display(value); }

/*************************************************************************************************/

template <class T>
struct ViewConcept
{
    typedef typename view_model_type<T>::type model_type; 

    static void display(T& view, const model_type& value)
    {
        using adobe::display; // pick up default version which looks for member functions
        display(view, value); // unqualified to allow user versions
    }

// Concept checking:
    //use pointers since not required to be default constructible
    T*  t; 
    typename view_model_type<T>::type* x;

    void constraints() {    
        // refinement of:
        // boost::function_requires<RegularConcept<T> >(); // not yet, views not yet regular
         //boost::function_requires<boost::CopyConstructibleConcept<T> >();

        // associated types:
        typedef typename view_model_type<T>::type associated_type;
        
        // operations:
        using adobe::display; // pick up default version which looks for member functions
        display(*t, *x);
    }
};

template <class T>
struct ViewConcept<boost::reference_wrapper<T> > : ViewConcept<T> 
{ 
    void constraints() {
        //boost concept check lib gets confused on VC8 without this
        ViewConcept<T>::constraints();
    }
};

#endif
// ADOBE_HAS_CPLUS0X_CONCEPTS

/*************************************************************************************************/

} //namespace adobe

/*************************************************************************************************/

#endif
