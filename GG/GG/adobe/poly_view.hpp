/*
    Copyright 2006-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_POLY_VIEW_HPP
#define ADOBE_POLY_VIEW_HPP

#include <GG/adobe/config.hpp>
#include <GG/adobe/view_concept.hpp>
#include <GG/adobe/poly.hpp>

/*************************************************************************************************/

namespace adobe {


/*************************************************************************************************/

/*!
  \brief Virtual interface for ViewConcept. Used in poly<view> implementation.
  \sa view, poly_view_instance, poly_copyable_interface, ViewConcept
*/
    
struct poly_view_interface : poly_copyable_interface
{
    virtual void display(const any_regular_t& new_value) = 0;
};

/*************************************************************************************************/

/*!
  \brief Implementation of \ref poly_view_interface in terms of types modeling \ref ViewConcept .
  Used in poly<view> implementation.
  \sa view, poly_view_interface, poly_copyable_interface, ViewConcept, optimized_storage_type
*/
    
template <typename T>
struct poly_view_instance : optimized_storage_type<T, poly_view_interface>::type
{
    typedef typename optimized_storage_type<T, poly_view_interface>::type base_t;

    /*!
        Check that T models appropriate concept for C++ 2003
    */
    BOOST_CLASS_REQUIRE(T, adobe, ViewConcept);

    /*!
        Construct from concrete value
    */
    poly_view_instance(const T& x) : base_t(x) {}

    /*!
        Move constructor
    */
    poly_view_instance(move_from<poly_view_instance> x) : base_t(move_from<base_t>(x.source)) {}

    void display(const any_regular_t& new_value)
    { 
        ViewConcept<T>::display(this->get(), new_value.cast<typename ViewConcept<T>::model_type>());
    }

}; 

/*************************************************************************************************/

/*!
  \brief "Handle" class used in poly<view> implementation.
  \sa poly_view_instance, poly_view_interface, poly_copyable_interface, ViewConcept, poly_base
*/

struct view : poly_base<poly_view_interface, poly_view_instance>
{
    typedef poly_base<poly_view_interface, poly_view_instance> base_t;

    /*!
        Construct from concrete value
    */
    template <typename T>
    explicit view(const T& s) : base_t(s)  { }

    /*!
        Move constructor
    */
    view(move_from<view> x) : base_t(move_from<base_t>(x.source)) {}

    template <typename V>
    void display(const V& new_value)
    { interface_ref().display(any_regular_t(new_value)); }
};


/*************************************************************************************************/

#if !defined(ADOBE_NO_DOCUMENTATION)

/*************************************************************************************************/

typedef poly<view> poly_view_t;

/*************************************************************************************************/

template <>
struct view_model_type<poly_view_t> 
{
    typedef any_regular_t type;
};


/*************************************************************************************************/

namespace implementation {

    
/*************************************************************************************************/

template <typename ArgType, typename Function>
struct function_as_view 
{ 
//  MM: To do: write Callable1Concept
//  BOOST_CLASS_REQUIRE2(Function, const ModelType&, adobe, Callable1Concept)

    typedef ArgType model_type;

    explicit function_as_view(const Function& f) : f_m(f) {}

    void display(const model_type& x) { f_m(x); }

// MM: hack so as to work with non-regular F's
    friend inline bool operator==(const function_as_view&, const function_as_view&)
    { return true; }

    Function f_m;

};

/*************************************************************************************************/

template <typename ArgType, typename Function>
inline function_as_view<ArgType, Function> make_function_as_view(const Function& f)
{
    return function_as_view<ArgType, Function>(f);
}


} //namespace implementation

/*************************************************************************************************/

#endif

/*************************************************************************************************/

/*!
  \brief Create an adapter to allow function-like objects to model ViewConcept.

  \pre  BOOST_CLASS_REQUIRE2(Function, const ArgType&, adobe, Callable1Concept)

example usage 

\code 
    void simple_display(cell_set_t::value_type& cell_m, 
                        const adobe::any_regular_t& new_value)
    //...
    std::vector<adobe::poly<adobe::view> > views;
    //...
    loop:
        views.push_back(make_function_as_poly_view<any_regular_t>(
                        boost::bind(&simple_display, cell_value, _1)));

        sheet_m.attach_view(iter->first, views.back());
    // etc.

\endcode
*/

template <typename ArgType, typename Function>
inline poly<view> make_function_as_poly_view(const Function& f)
{
    return poly<view>(implementation::function_as_view<ArgType, Function>(f));
}

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/

#endif
