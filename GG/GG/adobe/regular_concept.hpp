/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_REGULAR_HPP
#define ADOBE_REGULAR_HPP

/*************************************************************************************************/

#ifdef ADOBE_HAS_CPLUS0X_CONCEPTS

/*************************************************************************************************/

#include <concepts>

/*************************************************************************************************/

namespace adobe {
    
/*************************************************************************************************/

auto concept RegularConcept<typename T>
: std::CopyConstructible<T>, 
  std::Assignable<T>, 
  std::EqualityComparable<T>, 
  std::Swappable<T>,
  std::DefaultConstructible<T> // not yet
{
};

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/

#else

/*************************************************************************************************/

#include <boost/concept_check.hpp>

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/

template <class T>
struct RegularConcept
{

// Concept checking:

    void constraints() {
        // refinement of:
        boost::function_requires<boost::CopyConstructibleConcept<T> >();
        boost::function_requires<boost::AssignableConcept<T> >();
        boost::function_requires<boost::EqualityComparableConcept<T> >();
        boost::function_requires<boost::DefaultConstructibleConcept<T> >();
        //        boost::function_requires<boost::SwappableConcept<T> >();

        swap(t,t);
    }
#if !defined(ADOBE_NO_DOCUMENTATION)
    T t;
#endif
};

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/

#endif

/*************************************************************************************************/

#endif
