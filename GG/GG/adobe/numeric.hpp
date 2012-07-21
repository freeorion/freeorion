/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_NUMERIC_HPP
#define ADOBE_NUMERIC_HPP

#include <GG/adobe/config.hpp>

#include <iterator>
#include <numeric>

#include <boost/bind.hpp>

// REVISIT (sparent): dissable warnings for unused arguments in boost 1.32.0
#if defined(__MWERKS__)
#pragma warn_unusedarg off
#endif

#include <boost/range.hpp>

#if defined(__MWERKS__)
#pragma warn_unusedarg reset
#endif

namespace adobe {
/*!
\defgroup numeric Numeric Algorithms
\ingroup algorithm

\brief
Extends the C++ standard numeric algorithms with \ref concept_convertible_to_range and \ref concept_convertible_to_function concepts.

\note
These extensions reside in the <code>adobe::</code> namespace, not the <code>std::</code> namespace.
*/

/*!
\defgroup max_adjacent_difference max_adjacent_difference
\ingroup numeric
 */


/*************************************************************************************************/

/*!
\ingroup max_adjacent_difference
*/
template <typename ForwardIterator>
ForwardIterator max_adjacent_difference(ForwardIterator first, ForwardIterator last)
{
    typedef typename std::iterator_traits<ForwardIterator>::value_type value_type;
    
    ForwardIterator result(first);
    
    if (first == last) return result;
    
    ForwardIterator previous(first);
    ++first;
    
    if (first == last) return result;
    
    value_type result_difference(*first - *previous);
    previous = first; ++first;
    
    while (first != last)
    {
        value_type difference(*first - *previous);
        
        if (result_difference < difference)
        {
            result_difference = difference;
            result = previous;
        }
        previous = first;
        ++first;
    }
    
    return result;
}

/*!
\ingroup max_adjacent_difference
*/
template <typename ForwardRange>
inline typename boost::range_iterator<ForwardRange>::type
        max_adjacent_difference(ForwardRange& range)
{
    return adobe::max_adjacent_difference(boost::begin(range), boost::end(range));
}

/*!
\ingroup max_adjacent_difference
*/
template <typename ForwardRange>
inline typename boost::range_const_iterator<ForwardRange>::type
        max_adjacent_difference(const ForwardRange& range)
{
    return adobe::max_adjacent_difference(boost::begin(range), boost::end(range));
}

/*************************************************************************************************/

// standard calls using bind and range
    
/*************************************************************************************************/

/*!
\defgroup accumulate accumulate
\ingroup numeric
 */

/*!
\ingroup accumulate
\see
        STL documentation for \ref stldoc_accumulate
*/
template <typename InputRange, typename T>
inline T accumulate(const InputRange& range, T init)
{
    return std::accumulate(boost::begin(range), boost::end(range), init);
}

/*!
\ingroup accumulate
\see
        STL documentation for \ref stldoc_accumulate
*/
template <typename InputIterator, typename T, typename BinaryOperation>
inline T accumulate(InputIterator first, InputIterator last, T init, BinaryOperation binary_op)
{
    return std::accumulate(first, last, init, boost::bind(binary_op, _1, _2));
}

/*!
\ingroup accumulate
\see
        STL documentation for \ref stldoc_accumulate
*/
template <typename InputRange, typename T, typename BinaryOperation>
inline T accumulate(const InputRange& range, T init, BinaryOperation binary_op)
{
    return adobe::accumulate(boost::begin(range), boost::end(range), init, binary_op);
}

/*!
\defgroup inner_product inner_product
\ingroup numeric
 */

/*!
\ingroup inner_product
\see
        STL documentation for \ref inner_product
*/
template <typename InputRange, typename InputIterator, typename T>
inline T inner_product(const InputRange& range, InputIterator first, T init)
{
    return std::inner_product(boost::begin(range), boost::end(range), first, init);
}

/*!
\ingroup inner_product
\see
        STL documentation for \ref inner_product
*/
template <typename InputIterator1, typename InputIterator2, typename T,
          typename BinaryOperation1, typename BinaryOperation2>
inline T inner_product(InputIterator1 first1, InputIterator1 last1, InputIterator2 first2, T init,
        BinaryOperation1 binary_op1, BinaryOperation2 binary_op2)
{
    return std::inner_product(first1, last1, first2, init, boost::bind(binary_op1, _1, _2),
            boost::bind(binary_op2, _1, _2));
}

/*!
\ingroup inner_product
\see
        STL documentation for \ref inner_product
*/
template <typename InputRange, typename InputIterator, typename T,
          typename BinaryOperation1, typename BinaryOperation2>
inline T inner_product(const InputRange& range, InputIterator first, T init,
        BinaryOperation1 binary_op1, BinaryOperation2 binary_op2)
{
    return adobe::inner_product(boost::begin(range), boost::end(range), first, init, binary_op1,
        binary_op2);
}

/*!
\defgroup partial_sum partial_sum
\ingroup numeric
 */

/*!
\ingroup partial_sum
\see
        STL documentation for \ref stldoc_partial_sum
*/
template <typename InputRange, typename OutputIterator>
inline OutputIterator partial_sum(const InputRange& range, OutputIterator result)
{
    return std::partial_sum(boost::begin(range), boost::end(range), result);
}

/*!
\ingroup partial_sum
\see
        STL documentation for \ref stldoc_partial_sum
*/
template <typename InputIterator, typename OutputIterator, typename BinaryOperation>
inline OutputIterator partial_sum(InputIterator first, InputIterator last, OutputIterator result,
        BinaryOperation binary_op)
{
    return std::partial_sum(first, last, result, boost::bind(binary_op, _1, _2));
}

/*!
\ingroup partial_sum
\see
        STL documentation for \ref stldoc_partial_sum
*/
template <typename InputRange, typename OutputIterator, typename BinaryOperation>
inline OutputIterator partial_sum(const InputRange& range, OutputIterator result,
        BinaryOperation binary_op)
{
    return adobe::partial_sum(boost::begin(range), boost::end(range), result, binary_op);
}

/*!
\defgroup adjacent_difference adjacent_difference
\ingroup numeric
 */
/*!
\ingroup adjacent_difference
\see
        STL documentation for \ref stldoc_adjacent_difference
*/
template <typename InputRange, typename OutputIterator>
inline OutputIterator adjacent_difference(const InputRange& range, OutputIterator result)
{
    return std::adjacent_difference(boost::begin(range), boost::end(range), result);
}

/*!
\ingroup adjacent_difference
\see
        STL documentation for \ref stldoc_adjacent_difference
*/
template <typename InputIterator, typename OutputIterator, typename BinaryOperation>
inline OutputIterator adjacent_difference(InputIterator first, InputIterator last,
        OutputIterator result, BinaryOperation binary_op)
{
    return std::adjacent_difference(first, last, result, boost::bind(binary_op, _1, _2));
}

/*!
\ingroup adjacent_difference
\see
        STL documentation for \ref stldoc_adjacent_difference
*/
template <typename InputRange, typename OutputIterator, typename BinaryOperation>
inline OutputIterator adjacent_difference(const InputRange& range, OutputIterator result,
        BinaryOperation binary_op)
{
    return adobe::adjacent_difference(boost::begin(range), boost::end(range), result, binary_op);
}

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/

#endif

/*************************************************************************************************/
