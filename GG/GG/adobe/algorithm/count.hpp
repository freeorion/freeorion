/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_ALGORITHM_COUNT_HPP
#define ADOBE_ALGORITHM_COUNT_HPP

#include <GG/adobe/config.hpp>

#include <boost/range/begin.hpp>
#include <boost/range/difference_type.hpp>
#include <boost/range/end.hpp>
#include <boost/bind.hpp>

#include <algorithm>

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/
/*!
\defgroup count count
\ingroup non_mutating_algorithm

\see
    - STL documentation for \ref stldoc_count
    - STL documentation for \ref stldoc_count_if
*/
/*************************************************************************************************/
/*!
    \ingroup count

    \brief count implementation
*/
template <class InputRange, class T>
inline typename boost::range_difference<InputRange>::type count(InputRange& range, T& value)
{
    return std::count(boost::begin(range), boost::end(range), value);
}

/*!
    \ingroup count

    \brief count implementation
*/
template <class InputRange, class T>
inline typename boost::range_difference<InputRange>::type
count(const InputRange& range, T& value)
{
    return std::count(boost::begin(range), boost::end(range), value);
}

/*!
    \ingroup count

    \brief count implementation
*/
template <class InputIterator, class Predicate>
inline typename std::iterator_traits<InputIterator>::difference_type
count_if(InputIterator first, InputIterator last, Predicate pred)
{
    return std::count_if(first, last, boost::bind(pred, _1));
}

/*!
    \ingroup count

    \brief count implementation
*/
template <class InputRange, class Predicate>
inline typename boost::range_difference<InputRange>::type
count_if(InputRange& range, Predicate pred)
{
    return adobe::count_if(boost::begin(range), boost::end(range), pred);
}

/*!
    \ingroup count

    \brief count implementation
*/
template <class InputRange, class Predicate>
inline typename boost::range_difference<InputRange>::type
count_if(const InputRange& range, Predicate pred)
{
    return adobe::count_if(boost::begin(range), boost::end(range), pred);
}

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/

#endif

/*************************************************************************************************/
