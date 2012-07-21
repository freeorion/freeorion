/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_ALGORITHM_FIND_MATCH_HPP
#define ADOBE_ALGORITHM_FIND_MATCH_HPP

#include <GG/adobe/config.hpp>

#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <boost/bind.hpp>

#include <algorithm>

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/
/*!
\defgroup find_match find_match
\ingroup non_mutating_algorithm

Returns the first iterator \c i in the range <code>[first, last)</code> such that <code>comp(value,
*i)</code> return true. Returns last if no such iterator exists.

\complexity
Linear: at most <code>last - first</code> applications of \c comp.
*/
/*************************************************************************************************/
/*!
    \ingroup find_match

    \brief find_match implementation
*/
template <class InputIterator, class T, class Compare>
inline InputIterator
find_match(InputIterator first, InputIterator last, const T& value, Compare comp)
{
    return std::find_if(first, last, boost::bind(comp, value, _1));
}

/*!
    \ingroup find_match

    \brief find_match implementation
*/
template <class InputRange, class T, class Compare>
inline typename boost::range_iterator<InputRange>::type
find_match(InputRange& range, const T& value, Compare comp)
{
    return adobe::find_match(boost::begin(range), boost::end(range), value, comp);
}

/*!
    \ingroup find_match

    \brief find_match implementation
*/
template <class InputRange, class T, class Compare>
inline typename boost::range_const_iterator<InputRange>::type
find_match(const InputRange& range, const T& value, Compare comp)
{
    return adobe::find_match(boost::begin(range), boost::end(range), value, comp);
}

/*!
    \ingroup find_match

    \brief find_match implementation
*/
template <class InputIterator, class T, class Compare>
inline InputIterator find_match(InputIterator first, InputIterator last, const T& value)
{
    return std::find_if(first, last, boost::bind(std::equal_to<T>(), value, _1));
}

/*!
    \ingroup find_match

    \brief find_match implementation
*/
template <class InputRange, class T, class Compare>
inline typename boost::range_iterator<InputRange>::type
find_match(InputRange& range, const T& value)
{
    return adobe::find_match(boost::begin(range), boost::end(range), value);
}

/*!
    \ingroup find_match

    \brief find_match implementation
*/
template <class InputRange, class T, class Compare>
inline typename boost::range_const_iterator<InputRange>::type
find_match(const InputRange& range, const T& value)
{
    return adobe::find_match(boost::begin(range), boost::end(range), value);
}


/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/

#endif

/*************************************************************************************************/
