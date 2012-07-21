/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_ALGORITHM_EQUAL_RANGE_HPP
#define ADOBE_ALGORITHM_EQUAL_RANGE_HPP

#include <GG/adobe/config.hpp>

#include <algorithm>
#include <cassert>
#include <utility>

#include <boost/bind.hpp>
#include <boost/next_prior.hpp>
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>

#include <GG/adobe/algorithm/lower_bound.hpp>
#include <GG/adobe/algorithm/upper_bound.hpp>
#include <GG/adobe/functional.hpp>

/*************************************************************************************************/

/*!
\defgroup equal_range equal_range
\ingroup sorting

\see
    - STL documentation for \ref stldoc_equal_range
*/

/*************************************************************************************************/

namespace adobe {
namespace implementation {

/*************************************************************************************************/

template <  typename I, // I models ForwardIterator
            typename N, // N models IntegralType
            typename T, // T == result_type(P)
            typename C, // C models StrictWeakOrdering(T, T)
            typename P> // P models UnaryFunction(value_type(I)) -> T
std::pair<I, I> equal_range_n(I f, N n, const T& x, C c, P p)
{
    assert(!(n < 0) && "FATAL (sparent) : n must be >= 0");

    while (n != 0) {
        N h = n >> 1;
        I m = boost::next(f, h);

        if (c(p(*m), x)) {
            f = boost::next(m);
            n -= h + 1;
        } else if (c(x, p(*m))) {
            n = h;
        } else {
            return std::make_pair(implementation::lower_bound_n_(f, h, x, c, p),
                implementation::upper_bound_n(boost::next(m), n - (h + 1), x, c, p));
        }
    }
    return std::make_pair(f, f);
}

/*************************************************************************************************/

template <typename I> // I models ForwardRange
struct lazy_range
{
    typedef std::pair<typename boost::range_iterator<I>::type,
        typename boost::range_iterator<I>::type>    type;
};

/*************************************************************************************************/

template <typename I> // I models ForwardRange
struct lazy_range_const
{
    typedef std::pair<typename boost::range_const_iterator<I>::type,
        typename boost::range_const_iterator<I>::type>    type;
};

/*************************************************************************************************/

} // namespace implementation

/*************************************************************************************************/

/*!
    \ingroup equal_range
*/

template <  typename I, // I models ForwardIterator
            typename N, // N models IntegralType
            typename T> // T == result_type(P)
inline std::pair<I, I> equal_range_n(I f, N n, const T& x)
{
    return implementation::equal_range_n(f, n, x, less(), identity<T>());
}

/*************************************************************************************************/

/*!
    \ingroup equal_range
*/

template <  typename I, // I models ForwardIterator
            typename N, // N models IntegralType
            typename T, // T == result_type(P)
            typename C> // C models StrictWeakOrdering(T, T)
inline std::pair<I, I> equal_range_n(I f, N n, const T& x, C c)
{
    return implementation::equal_range_n(f, n, x, boost::bind(c, _1, _2), identity<T>());
}

/*************************************************************************************************/

/*!
    \ingroup equal_range
*/

template <  typename I, // I models ForwardIterator
            typename N, // N models IntegralType
            typename T, // T == result_type(P)
            typename C, // C models StrictWeakOrdering(T, T)
            typename P> // P models UnaryFunction(value_type(I)) -> T
inline std::pair<I, I> equal_range_n(I f, N n, const T& x, C c, P p)
{
    return implementation::equal_range_n(f, n, x, boost::bind(c, _1, _2), boost::bind(p, _1));
}

/*************************************************************************************************/

/*!
    \ingroup equal_range
*/

template <  typename I, // I models ForwardIterator
            typename T> // T == value_type(I)
inline std::pair<I, I> equal_range(I f, I l, const T& x)
{
    return std::equal_range(f, l, x);
}

/*************************************************************************************************/

/*!
    \ingroup equal_range
*/

template <  typename I, // I models ForwardIterator
            typename T, // T == result_type(P)
            typename C> // C models StrictWeakOrdering(T, T)
inline std::pair<I, I> equal_range(I f, I l, const T& x, C c)
{
    return std::equal_range(f, l, x, boost::bind(c, _1, _2));
}

/*************************************************************************************************/

/*!
    \ingroup equal_range
*/

template <  typename I, // I models ForwardIterator
            typename T, // T == result_type(P)
            typename C, // C models StrictWeakOrdering(T, T)
            typename P> // P models UnaryFunction(value_type(I)) -> T
inline std::pair<I, I> equal_range(I f, I l, const T& x, C c, P p)
{
    return equal_range_n(f, std::distance(f, l), x, c, p);
}

/*************************************************************************************************/

/*!
    \ingroup equal_range
*/

template <  typename I, // I models ForwardRange
            typename T, // T == result_type(P)
            typename C, // C models StrictWeakOrdering(T, T)
            typename P> // P models UnaryFunction(value_type(I)) -> T
inline
typename boost::lazy_disable_if<boost::is_same<I, T>, implementation::lazy_range<I> >::type
        equal_range(I& r, const T& x, C c, P p)
{ return adobe::equal_range(boost::begin(r), boost::end(r), x, c, p); }

/*************************************************************************************************/

/*!
    \ingroup equal_range
*/

template <  typename I, // I models ForwardRange
            typename T, // T == result_type(P)
            typename C, // C models StrictWeakOrdering(T, T)
            typename P> // P models UnaryFunction(value_type(I)) -> T
inline
typename boost::lazy_disable_if<boost::is_same<I, T>, implementation::lazy_range_const<I> >::type
        equal_range(const I& r, const T& x, C c, P p)
{ return adobe::equal_range(boost::begin(r), boost::end(r), x, c, p); }

/*************************************************************************************************/

/*!
    \ingroup equal_range
*/

template <  typename I, // I models ForwardRange
            typename T> // T == result_type(P)
inline std::pair<typename boost::range_iterator<I>::type,
                 typename boost::range_iterator<I>::type>
equal_range(I& r, const T& x)
{
    return std::equal_range(boost::begin(r), boost::end(r), x);
}

/*************************************************************************************************/

/*!
    \ingroup equal_range
*/

template <  typename I, // I models ForwardRange
            typename T> // T == result_type(P)
inline std::pair<typename boost::range_const_iterator<I>::type,
                 typename boost::range_const_iterator<I>::type>
equal_range(const I& r, const T& x)
{
    return std::equal_range(boost::begin(r), boost::end(r), x);
}

/*************************************************************************************************/

/*!
    \ingroup equal_range
*/

template <  typename I, // I models ForwardRange
            typename T, // T == result_type(P)
            typename C> // C models StrictWeakOrdering(T, T)
inline
typename boost::lazy_disable_if<boost::is_same<I, T>, implementation::lazy_range<I> >::type
equal_range(I& r, const T& x, C c)
{
    return adobe::equal_range(boost::begin(r), boost::end(r), x, c);
}

/*************************************************************************************************/

/*!
    \ingroup equal_range
*/

template <  typename I, // I models ForwardRange
            typename T, // T == result_type(P)
            typename C> // C models StrictWeakOrdering(T, T)
inline
typename boost::lazy_disable_if<boost::is_same<I, T>, implementation::lazy_range_const<I> >::type
equal_range(const I& r, const T& x, C c)
{
    return adobe::equal_range(boost::begin(r), boost::end(r), x, c);
}

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/

#endif

/*************************************************************************************************/
