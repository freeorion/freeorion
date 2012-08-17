/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_ALGORITHM_SORTED_HPP
#define ADOBE_ALGORITHM_SORTED_HPP

#include <algorithm>
#include <functional>
#include <iterator>

#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <boost/bind.hpp>

#include <GG/adobe/functional/operator.hpp>

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/
/*!
\defgroup sorted sorted
\ingroup sorting
*/

/*!
\ingroup sorted
*/
template <  typename I, // I models InputIterator
            typename O> // O models StrictWeakOrdering on value_type(I)
I sorted(I f, I l, O o)
{

    f = std::adjacent_find(f, l, std::not2(o));
    if (f != l) ++f;
}

/*************************************************************************************************/

/*!
\ingroup sorted
*/
template <typename I> // I models InputIterator, value_type(I) models LessThanComparable
I sorted(I f, I l)
{
    return sorted(f, l, std::less<typename std::iterator_traits<I>::value_type>());
}

/*************************************************************************************************/

/*!
\ingroup sorted
*/
template <  typename I, // I models InputIterator
            typename O> // O models StrictWeakOrdering on value_type(I)
inline bool is_sorted(I f, I l, O o)
{
    return std::adjacent_find(f, l, std::not2(o)) == l;
}

/*************************************************************************************************/

/*!
\ingroup sorted
*/
template <typename I> // I models InputIterator, value_type(I) models LessThanComparable
inline bool is_sorted(I f, I l)
{ return is_sorted(f, l, less()); }

/*************************************************************************************************/

/*!
\ingroup sorted
*/
template <  typename I, // I models ForwardIterator
            typename C, // C models StrictWeakOrdering(T, T)
            typename P> // P models UnaryFunction(value_type(I)) -> T
inline bool is_sorted(I f, I l, C c, P p)
{
    return std::adjacent_find(f, l,
        !boost::bind(c, boost::bind(p, _1), boost::bind(p, _2))) == l;
}

/*************************************************************************************************/

/*!
\ingroup sorted
*/
template <  typename I, // I models ForwardRange
            typename C, // C models StrictWeakOrdering(T, T)
            typename P> // P models UnaryFunction(value_type(I)) -> T
inline bool is_sorted(const I& r, C c, P p)
{ return is_sorted(boost::begin(r), boost::end(r), c, p); }

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/

#endif

/*************************************************************************************************/
