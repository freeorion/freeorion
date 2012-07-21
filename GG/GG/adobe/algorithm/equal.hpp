/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_ALGORITHM_EQUAL_HPP
#define ADOBE_ALGORITHM_EQUAL_HPP

#include <GG/adobe/config.hpp>

#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <boost/bind.hpp>

#include <algorithm>

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/
/*!
\defgroup equal equal
\ingroup non_mutating_algorithm

\see
    - STL documentation for \ref stldoc_equal
*/
/*************************************************************************************************/
/*!
    \ingroup equal
*/
template <class InputIterator1, class InputIterator2, class BinaryPredicate>
inline bool
equal(InputIterator1 first1, InputIterator1 last1, InputIterator2 first2, BinaryPredicate pred)
{
    return std::equal(first1, last1, first2, boost::bind(pred, _1, _2));
}

/*!
    \ingroup equal
*/
template <class InputRange1, class InputIterator2>
inline bool equal(const InputRange1& range1, InputIterator2 first2)
{
    return std::equal(boost::begin(range1), boost::end(range1), first2);
}

/*!
    \ingroup equal
*/
template <class InputRange1, class InputIterator2, class BinaryPredicate>
inline bool equal(const InputRange1& range1, InputIterator2 first2, BinaryPredicate pred)
{
    return adobe::equal(boost::begin(range1), boost::end(range1), first2, pred);
}

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/

#endif

/*************************************************************************************************/
