/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_ALGORITHM_LOWER_BOUND_HPP
#define ADOBE_ALGORITHM_LOWER_BOUND_HPP

#include <GG/adobe/config.hpp>

#include <algorithm>
#include <iterator>

#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <boost/bind.hpp>
#include <boost/next_prior.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/utility/enable_if.hpp>

#include <GG/adobe/functional/operator.hpp>

/*************************************************************************************************/

namespace adobe {
namespace implementation {

/*************************************************************************************************/

template <  typename I, // I models ForwardIterator
            typename N, // N models IntegralType
            typename T, // T == result_type(P)
            typename C, // C models StrictWeakOrdering(T, T)
            typename P> // P models UnaryFunction(value_type(I)) -> T
I lower_bound_n_(I f, N n, const T& x, C c, P p)
{
    while (n != 0)
    {
        N h = n >> 1;
        I m = boost::next(f, h);

        if (c(p(*m), x)) { f = boost::next(m); n -= h + N(1); }
        else { n = h; }
    }
    return f;
}

/*************************************************************************************************/

} // implementation

/*************************************************************************************************/

/*!
\defgroup lower_bound lower_bound
\ingroup sorting

\see
    - STL documentation for \ref stldoc_lower_bound
*/

/*************************************************************************************************/

template <  typename I, // I models ForwardIterator
            typename N, // N models IntegralType
            typename T> // T == value_type(I)
inline I lower_bound_n(I f, N n, const T& x)
{
    return implementation::lower_bound_n_(f, n, x, less(), identity<T>());
}

/*************************************************************************************************/

template <  typename I, // I models FowardIterator
            typename N, // N models IntegralType
            typename T, // T == value_type(I)
            typename C> // C models StrictWeakOrdering(T, T)
inline I lower_bound_n(I f, N n, const T& x, C c)
{
    return implementation::lower_bound_n_(f, n, x, boost::bind(c, _1, _2), identity<T>());
}

/*************************************************************************************************/

template <  typename I, // I models ForwardIterator
            typename N, // N models IntegralType
            typename T, // T == result_type(P)
            typename C, // C models StrictWeakOrdering(T, T)
            typename P> // P models UnaryFunction(value_type(I)) -> T
inline I lower_bound_n(I f, N n, const T& x, C c, P p)
{
    return implementation::lower_bound_n_(f, n, x, boost::bind(c, _1, _2), boost::bind(p, _1));
}

/*************************************************************************************************/

/*
    NOTE (sparent) : These functions collide with the std functions when called unqualified as
    is done by std::equal_range with VC++. We hide them from ADL by moving them into the
    fn namespace.
*/

namespace fn { } using namespace fn;
namespace fn {

/*************************************************************************************************/

template <  typename I, // I models ForwardIterator
            typename T> // T == value_type(I)
inline I lower_bound(I f, I l, const T& x)
{
    return std::lower_bound(f, l, x);
}

/*************************************************************************************************/

template <  typename I, // I models FowardIterator
            typename T, // T == value_type(I)
            typename C> // C models StrictWeakOrdering(T, T)
inline I lower_bound(I f, I l, const T& x, C c)
{
    return std::lower_bound(f, l, x, boost::bind(c, _1, _2));
}

/*************************************************************************************************/

template <  typename I, // I models ForwardIterator
            typename T, // T == result_type(P)
            typename C, // C models StrictWeakOrdering(T, T)
            typename P> // P models UnaryFunction(value_type(I)) -> T
inline I lower_bound(I f, I l, const T& x, C c, P p)
{
    return lower_bound_n(f, std::distance(f, l), x, c, p);
}

/*************************************************************************************************/

template <  typename I, // I models ForwardRange
            typename T, // T == result_type(P)
            typename C, // C models StrictWeakOrdering(T, T)
            typename P> // P models UnaryFunction(value_type(I)) -> T
inline
    typename boost::lazy_disable_if<boost::is_same<I, T>, boost::range_iterator<I> >::type
        lower_bound(I& r, const T& x, C c, P p)
{ return adobe::lower_bound(boost::begin(r), boost::end(r), x, c, p); }

/*************************************************************************************************/

template <  typename I, // I models ForwardRange
            typename T, // T == result_type(P)
            typename C, // C models StrictWeakOrdering(T, T)
            typename P> // P models UnaryFunction(value_type(I)) -> T
inline
    typename boost::lazy_disable_if<boost::is_same<I, T>, boost::range_const_iterator<I> >::type
        lower_bound(const I& r, const T& x, C c, P p)
{ return adobe::lower_bound(boost::begin(r), boost::end(r), x, c, p); }

/*************************************************************************************************/
/*!
    \ingroup lower_bound

    \brief lower_bound implementation
*/
template <class ForwardRange, class T>
inline typename boost::range_iterator<ForwardRange>::type lower_bound(ForwardRange& range, const T& value)
{
    return std::lower_bound(boost::begin(range), boost::end(range), value);
}

/*!
    \ingroup lower_bound

    \brief lower_bound implementation
*/
template <class ForwardRange, class T>
inline typename boost::range_const_iterator<ForwardRange>::type lower_bound(const ForwardRange& range, const T& value)
{
    return std::lower_bound(boost::begin(range), boost::end(range), value);
}

/*!
    \ingroup lower_bound

    \brief lower_bound implementation
*/
/*!
    \ingroup lower_bound

    \brief lower_bound implementation
*/
template <typename I, class T, class Compare>
inline typename boost::lazy_disable_if<boost::is_same<I, T>, boost::range_iterator<I> >::type
        lower_bound(I& range, const T& value, Compare comp)
{
    return adobe::lower_bound(boost::begin(range), boost::end(range), value, comp);
}

/*!
    \ingroup lower_bound

    \brief lower_bound implementation
*/
template <class I, class T, class Compare>
inline typename boost::lazy_disable_if<boost::is_same<I, T>, boost::range_const_iterator<I> >::type
        lower_bound(const I& range, const T& value, Compare comp)
{
    return adobe::lower_bound(boost::begin(range), boost::end(range), value, comp);
}

/*************************************************************************************************/

} // namespace fn
} // namespace adobe

/*************************************************************************************************/

#endif

/*************************************************************************************************/
