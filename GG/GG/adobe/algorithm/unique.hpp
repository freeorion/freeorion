/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_ALGORITHM_UNIQUE_HPP
#define ADOBE_ALGORITHM_UNIQUE_HPP

#include <GG/adobe/config.hpp>

#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <boost/bind.hpp>

#include <algorithm>

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/
/*!
\defgroup unique unique
\ingroup mutating_algorithm

\see
    - STL documentation for \ref stldoc_unique
    - STL documentation for \ref stldoc_unique_copy
*/
/*************************************************************************************************/
/*!
    \ingroup unique

    \brief unique implementation
*/
template <class ForwardRange>
inline typename boost::range_iterator<ForwardRange>::type unique(ForwardRange& range)
{
    return std::unique(boost::begin(range), boost::end(range));
}

/*!
    \ingroup unique

    \brief unique implementation
*/
template <class ForwardIterator, class BinaryPredicate>
inline ForwardIterator unique(ForwardIterator first, ForwardIterator last, BinaryPredicate pred)
{
    return std::unique(first, last, boost::bind(pred, _1, _2));
}

/*!
    \ingroup unique

    \brief unique implementation
*/
template <class ForwardRange, class BinaryPredicate>
inline typename boost::range_iterator<ForwardRange>::type unique(ForwardRange& range, BinaryPredicate pred)
{
    return adobe::unique(boost::begin(range), boost::end(range), pred);
}

/*!
    \ingroup unique

    \brief unique implementation
*/
template <class InputRange, class OutputIterator>
inline OutputIterator unique_copy(InputRange& range, OutputIterator result)
{
    return std::unique_copy(boost::begin(range), boost::end(range), result);
}

/*!
    \ingroup unique

    \brief unique implementation
*/
template <class InputRange, class OutputIterator>
inline OutputIterator unique_copy(const InputRange& range, OutputIterator result)
{
    return std::unique_copy(boost::begin(range), boost::end(range), result);
}

/*!
    \ingroup unique

    \brief unique implementation
*/
template <class InputIterator, class OutputIterator, class BinaryPredicate>
inline OutputIterator unique_copy(InputIterator first, InputIterator last, OutputIterator result, BinaryPredicate pred)
{
    return std::unique_copy(first, last, result, boost::bind(pred, _1, _2));
}

/*!
    \ingroup unique

    \brief unique implementation
*/
template <class InputRange, class OutputIterator, class BinaryPredicate>
inline OutputIterator unique_copy(InputRange& range, OutputIterator result, BinaryPredicate pred)
{
    return adobe::unique_copy(boost::begin(range), boost::end(range), result, pred);
}

/*!
    \ingroup unique

    \brief unique implementation
*/
template <class InputRange, class OutputIterator, class BinaryPredicate>
inline OutputIterator unique_copy(const InputRange& range, OutputIterator result, BinaryPredicate pred)
{
    return adobe::unique_copy(boost::begin(range), boost::end(range), result, pred);
}

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/

#endif

/*************************************************************************************************/
