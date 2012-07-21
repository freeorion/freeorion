/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_ALGORITHM_MINMAX_HPP
#define ADOBE_ALGORITHM_MINMAX_HPP

#include <GG/adobe/config.hpp>

#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <boost/bind.hpp>

#include <algorithm>

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/
/*!
\defgroup minmax min / max
\ingroup sorting

\note adobe::max differs is stable with respect to sort order where std::max is not. That is:

<code>
int a = 1;
int b = 1;

assert(&adobe::max(a, b) == b);
assert(&std::max(a, b) == a);
</code>

adobe::min and adobe::max also implement non-const forms which are not available in the standard
which allow the result to be used as an l-value.

<code>
int a = 1;
int b = 2;

adobe::min(a, b) = 10;

assert(a == 10);
</code>

\see
    - STL documentation for \ref stldoc_min
    - STL documentation for \ref stldoc_max
    - STL documentation for \ref stldoc_min_element
    - STL documentation for \ref stldoc_max_element
*/
/*************************************************************************************************/

/*!
    \ingroup minmax

    \brief minmax implementation
*/

template <class T>
inline const T& (min)(const T& a, const T& b)
{ return a < b ? a : b; }

/*!
    \ingroup minmax

    \brief minmax implementation
*/

template <class T>
inline T& (min)(T& a, T& b)
{ return a < b ? a : b; }

/*!
    \ingroup minmax

    \brief minmax implementation
*/

template <class T>
inline const T& (max)(const T& a, const T& b)
{ return b < a ? a : b; }

/*!
    \ingroup minmax

    \brief minmax implementation
*/

template <class T>
inline T& (max)(T& a, T& b)
{ return b < a ? a : b; }

/*!
    \ingroup minmax

    \brief minmax implementation
*/

template <class T, class Compare>
inline const T& (min)(const T& a, const T& b, Compare comp)
{
    return (std::min)(a, b, boost::bind(comp, _1, _2));
}

/*!
    \ingroup minmax

    \brief minmax implementation
*/

template <class T, class Compare>
inline T& (min)(T& a, T& b, Compare comp)
{
    return (std::min)(a, b, boost::bind(comp, _1, _2));
}

/*!
    \ingroup minmax

    \brief minmax implementation
*/

template <class T, class Compare>
inline const T& (max)(const T& a, const T& b, Compare comp)
{
    return boost::bind(comp, _1, _2)(b, a) ? a : b;
}

/*!
    \ingroup minmax

    \brief minmax implementation
*/

template <class T, class Compare>
inline T& (max)(T& a, T& b, Compare comp)
{
    return boost::bind(comp, _1, _2)(b, a) ? a : b;
}

/*!
    \ingroup minmax

    \brief minmax implementation
*/
template <class ForwardRange>
inline typename boost::range_iterator<ForwardRange>::type min_element(ForwardRange& range)
{
    return std::min_element(boost::begin(range), boost::end(range));
}

/*!
    \ingroup minmax

    \brief minmax implementation
*/
template <class ForwardRange>
inline typename boost::range_const_iterator<ForwardRange>::type
min_element(const ForwardRange& range)
{
    return std::min_element(boost::begin(range), boost::end(range));
}

/*!
    \ingroup minmax

    \brief minmax implementation
*/
template <class ForwardIterator, class Compare>
inline ForwardIterator min_element(ForwardIterator first, ForwardIterator last, Compare comp)
{
    return std::min_element(first, last, boost::bind(comp, _1, _2));
}

/*!
    \ingroup minmax

    \brief minmax implementation
*/
template <class ForwardRange, class Compare>
inline typename boost::range_iterator<ForwardRange>::type
min_element(ForwardRange& range, Compare comp)
{
    return adobe::min_element(boost::begin(range), boost::end(range), comp);
}

/*!
    \ingroup minmax

    \brief minmax implementation
*/
template <class ForwardRange, class Compare>
inline typename boost::range_const_iterator<ForwardRange>::type
min_element(const ForwardRange& range, Compare comp)
{
    return adobe::min_element(boost::begin(range), boost::end(range), comp);
}

/*!
    \ingroup minmax

    \brief minmax implementation
*/
template <class ForwardRange>
inline typename boost::range_iterator<ForwardRange>::type max_element(ForwardRange& range)
{
    return std::max_element(boost::begin(range), boost::end(range));
}

/*!
    \ingroup minmax

    \brief minmax implementation
*/
template <class ForwardRange>
inline typename boost::range_const_iterator<ForwardRange>::type
max_element(const ForwardRange& range)
{
    return std::max_element(boost::begin(range), boost::end(range));
}

/*!
    \ingroup minmax

    \brief minmax implementation
*/
template <class ForwardIterator, class Compare>
inline ForwardIterator max_element(ForwardIterator first, ForwardIterator last, Compare comp)
{
    return std::max_element(first, last, boost::bind(comp, _1, _2));
}

/*!
    \ingroup minmax

    \brief minmax implementation
*/
template <class ForwardRange, class Compare>
inline typename boost::range_iterator<ForwardRange>::type
max_element(ForwardRange& range, Compare comp)
{
    return adobe::max_element(boost::begin(range), boost::end(range), comp);
}

/*!
    \ingroup minmax

    \brief minmax implementation
*/
template <class ForwardRange, class Compare>
inline typename boost::range_const_iterator<ForwardRange>::type
max_element(const ForwardRange& range, Compare comp)
{
    return adobe::max_element(boost::begin(range), boost::end(range), comp);
}

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/

#endif

/*************************************************************************************************/
