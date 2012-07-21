/*
    Copyright 2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/**************************************************************************************************/

#ifndef ADOBE_POLY_DRAG_AND_DROP_CONVERTER_HPP
#define ADOBE_POLY_DRAG_AND_DROP_CONVERTER_HPP

/**************************************************************************************************/

#include <GG/adobe/config.hpp>

#include <GG/adobe/any_regular.hpp>
#include <GG/adobe/poly.hpp>
#include <GG/adobe/future/drag_and_drop_converter_concept.hpp>

#include <boost/bind.hpp>
#include <boost/type_traits/function_traits.hpp>
#include <boost/type_traits/remove_pointer.hpp>
#include <boost/type_traits/remove_reference.hpp>
#include <boost/type_traits/remove_const.hpp>

/**************************************************************************************************/

namespace adobe {

/**************************************************************************************************/

struct poly_drag_and_drop_converter_interface : poly_copyable_interface
{
    virtual any_regular_t convert(const any_regular_t& new_value) const = 0;
};

/**************************************************************************************************/

template <typename T>
struct poly_drag_and_drop_converter_instance : optimized_storage_type<T, poly_drag_and_drop_converter_interface>::type
{
    typedef typename optimized_storage_type<T, poly_drag_and_drop_converter_interface>::type base_t;

    BOOST_CLASS_REQUIRE(T, adobe, DragAndDropConverterConcept);

    poly_drag_and_drop_converter_instance(move_from<poly_drag_and_drop_converter_instance> x) :
        base_t(move_from<base_t>(x.source))
    { }

    poly_drag_and_drop_converter_instance(const T& x) :
        base_t(x)
    { }

    any_regular_t convert(const any_regular_t& new_value) const
    {
        typedef typename DragAndDropConverterConcept<T>::source_type  const_source_type;
        typedef typename boost::remove_const<const_source_type>::type source_type;
        typedef typename DragAndDropConverterConcept<T>::dest_type    const_dest_type;
        typedef typename boost::remove_const<const_dest_type>::type   dest_type;

        return any_regular_t(DragAndDropConverterConcept<T>::convert(this->get(),
                                                                     new_value.cast<source_type>()));
    }
};

/**************************************************************************************************/

struct drag_and_drop_converter : poly_base<poly_drag_and_drop_converter_interface,
                                           poly_drag_and_drop_converter_instance>
{
    typedef poly_base<poly_drag_and_drop_converter_interface,
                                           poly_drag_and_drop_converter_instance> base_t;

    template <typename T>
    explicit drag_and_drop_converter(const T& s) :
        base_t(s)
        { }

    drag_and_drop_converter(move_from<drag_and_drop_converter> x)
        : base_t(move_from<base_t>(x.source)) {}

    template <typename V>
    any_regular_t convert(const V& new_value) const
    { return interface_ref().convert(any_regular_t(new_value)); }
};

/**************************************************************************************************/

typedef poly<drag_and_drop_converter> poly_drag_and_drop_converter_t;

/**************************************************************************************************/

#if !defined(ADOBE_NO_DOCUMENTATION)

/**************************************************************************************************/

namespace implementation {

/**************************************************************************************************/

template <typename Function>
struct function_as_drag_and_drop_converter 
{
    typedef typename boost::remove_pointer<Function>::type              function_type;
    typedef typename boost::function_traits<function_type>::result_type result_type;
    typedef typename boost::function_traits<function_type>::arg1_type   arg_type;
    typedef result_type                                                 dest_type;
    typedef typename boost::remove_reference<arg_type>::type            source_type;

//  MM: To do: write Callable1Concept
//  BOOST_CLASS_REQUIRE2(Function, const source_type&, adobe, Callable1Concept)

    explicit function_as_drag_and_drop_converter(const Function& f) : f_m(f) { }

    inline dest_type convert(const source_type& x) const { return f_m(x); }

// MM: hack so as to work with non-regular F's
    friend inline bool operator==(const function_as_drag_and_drop_converter&,
                                  const function_as_drag_and_drop_converter&)
    { return true; }

    Function f_m;
};

/**************************************************************************************************/

} //namespace implementation

/**************************************************************************************************/

#endif

/**************************************************************************************************/
/*!
  \brief Create an adapter to allow function-like objects to model DragAndDropConverterConcept.
*/
template <typename Function>
inline poly_drag_and_drop_converter_t make_function_as_poly_drag_and_drop_converter(const Function& f)
{
    return poly_drag_and_drop_converter_t(
        implementation::function_as_drag_and_drop_converter<Function>(f));
}

/**************************************************************************************************/

} // namespace adobe

/**************************************************************************************************/

// ADOBE_POLY_DRAG_AND_DROP_CONVERTER_HPP
#endif

/**************************************************************************************************/
