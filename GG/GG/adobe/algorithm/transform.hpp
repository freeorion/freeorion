/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_ALGORITHM_TRANSFORM_HPP
#define ADOBE_ALGORITHM_TRANSFORM_HPP

#include <GG/adobe/config.hpp>

#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <boost/bind.hpp>

#include <algorithm>

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/
/*!
\defgroup transform transform
\ingroup mutating_algorithm

\see
    - STL documentation for \ref stldoc_transform
*/
/*************************************************************************************************/
/*!
    \ingroup transform

    \brief transform implementation
*/
template <class InputIterator, class OutputIterator, class UnaryOperation>
inline OutputIterator transform(InputIterator first, InputIterator last, OutputIterator result,
    UnaryOperation op)
{
    return std::transform(first, last, result, boost::bind(op, _1));
}

/*!
    \ingroup transform

    \brief transform implementation
*/
template <class InputRange, class OutputIterator, class UnaryOperation>
inline OutputIterator transform(InputRange& range, OutputIterator result, UnaryOperation op)
{
    return adobe::transform(boost::begin(range), boost::end(range), result, op);
}
    

/*!
    \ingroup transform

    \brief transform implementation
*/
template <class InputRange, class OutputIterator, class UnaryOperation>
inline OutputIterator transform(const InputRange& range, OutputIterator result, UnaryOperation op)
{
    return adobe::transform(boost::begin(range), boost::end(range), result, op);
}
    

/*!
    \ingroup transform

    \brief transform implementation
*/
template <class InputIterator1, class InputIterator2, class OutputIterator, class BinaryOperation>
inline OutputIterator transform(InputIterator1 first1, InputIterator1 last1, InputIterator2 first2,
                                OutputIterator result, BinaryOperation binary_op)
{
    return std::transform(  first1, last1, first2, result,
                            boost::bind(binary_op, _1, _2));
}

/*!
    \ingroup transform

    \brief transform implementation
*/
template <class InputRange1, class InputIterator2, class OutputIterator, class BinaryOperation>
inline OutputIterator transform(InputRange1& range1, InputIterator2 first2, OutputIterator result,
                                BinaryOperation binary_op)
{
    return adobe::transform(boost::begin(range1), boost::end(range1), first2, result, binary_op);
}
    
/*!
    \ingroup transform

    \brief transform implementation
*/
template <class InputRange1, class InputIterator2, class OutputIterator, class BinaryOperation>
inline OutputIterator transform(const InputRange1& range1, InputIterator2 first2,
                                OutputIterator result, BinaryOperation binary_op)
{
    return adobe::transform(boost::begin(range1), boost::end(range1), first2, result, binary_op);
}

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/

#endif

/*************************************************************************************************/
