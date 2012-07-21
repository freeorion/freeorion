/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_CONTROLLER_HPP
#define ADOBE_CONTROLLER_HPP

/*************************************************************************************************/

#include <boost/concept_check.hpp>
#include <boost/function.hpp>

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/

template <class C>
struct controller_model_type 
{
    typedef typename C::model_type type;
};

/*************************************************************************************************/

template <class C, class F> // C Models Controller, F models UnaryFunction
inline void monitor(C& c, const F& setter)
{ c.monitor(setter); }

/*************************************************************************************************/

template <class C> // C Models Controller
inline void enable(C& c, bool enable_state)
{ c.enable(enable_state); }

/*************************************************************************************************/

template <class T>
struct ControllerConcept
{
    typedef typename controller_model_type<T>::type model_type; 

     
    template <class F>
    static void monitor(T& controller, F setter)
    {
        using adobe::monitor; // pick up default version which looks for member functions
        monitor(controller, setter); // unqualified to allow user versions
    }

    static void enable(T& controller, bool enable_state)
    {
        using adobe::enable; // pick up default version which looks for member functions
        enable(controller, enable_state); // unqualified to allow user versions
    }

// Concept checking:
    //use pointers since not required to be default constructible
    T* t; 
    bool b;
    boost::function<void (const typename controller_model_type<T>::type&)> f;

    void constraints() {
        // refinement of:
        //boost::function_requires<boost::CopyConstructibleConcept<T> >();
          // we can enable the CopyConstructible (or Regular) requirement
          // once widget types model it

        // associated types:
        typedef typename controller_model_type<T>::type associated_type;
        
        // operations:
        using adobe::enable; // pick up default version which looks for member functions
        enable(*t, b);

        using adobe::monitor; // pick up default version which looks for member functions
        monitor(*t, f);
    }
};

template <class T>
struct ControllerConcept<T*> : ControllerConcept<T> 
{
    static void enable(T* c, bool enable_state)
    { ControllerConcept<T>::enable(*c, enable_state); }

    static void monitor(T* c,  boost::function<void (const typename ControllerConcept<T>::model_type&)> setter)
    { ControllerConcept<T>::monitor(*c, setter); }

    void constraints() {
        //boost concept check lib gets confused on VC8 without this
        ControllerConcept<T>::constraints();
    }
};

/*************************************************************************************************/

} //namespace adobe

/*************************************************************************************************/

#endif
