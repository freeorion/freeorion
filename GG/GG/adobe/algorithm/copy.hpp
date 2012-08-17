/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_ALGORITHM_COPY_HPP
#define ADOBE_ALGORITHM_COPY_HPP

#include <GG/adobe/config.hpp>

#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <boost/range/size.hpp>

#include <algorithm>
#include <iterator>

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/
/*!
\defgroup copy copy
\ingroup mutating_algorithm

\see
    - STL documentation for \ref stldoc_copy
    - STL documentation for \ref stldoc_copy_n
    - STL documentation for \ref stldoc_copy_backward
*/
/*************************************************************************************************/

/*!
    \ingroup copy

    \brief copy implementation
*/
template <class InputRange, class OutputIterator>
inline OutputIterator copy(const InputRange& range, OutputIterator result)
{
    return std::copy(boost::begin(range), boost::end(range), result);
}

/*!
    \ingroup copy

    \brief copy implementation
*/
template <class BidirectionalRange1, class BidirectionalIterator2>
inline BidirectionalIterator2 copy_backward(BidirectionalRange1& range1, BidirectionalIterator2 result)
{
    return std::copy_backward(boost::begin(range1), boost::end(range1), result);
}

/*!
    \ingroup copy

    \brief copy implementation
*/
template <class BidirectionalRange1, class BidirectionalIterator2>
inline BidirectionalIterator2 copy_backward(const BidirectionalRange1& range1, BidirectionalIterator2 result)
{
    return std::copy_backward(boost::begin(range1), boost::end(range1), result);
}

/*************************************************************************************************/
#ifndef ADOBE_NO_DOCUMENTATION
namespace implementation {

/*************************************************************************************************/
/*!
    \ingroup copy
    \brief taken from SGI STL.
*/
template <class InputIter, class Size, class OutputIter>
std::pair<InputIter, OutputIter> copy_n(InputIter first, Size count,
                                        OutputIter result,
                                        std::input_iterator_tag) 
{
   for ( ; count > 0; --count) {
      *result = *first;
      ++first;
      ++result;
   }
   return std::pair<InputIter, OutputIter>(first, result);
}

/*!
    \ingroup copy

    \brief copy implementation
*/
template <class RAIter, class Size, class OutputIter>
inline std::pair<RAIter, OutputIter>
copy_n(RAIter first, Size count, OutputIter result, std::random_access_iterator_tag) 
{
   RAIter last = first + count;
   return std::pair<RAIter, OutputIter>(last, std::copy(first, last, result));
}

/*************************************************************************************************/

} // namespace implementation
#endif
/*************************************************************************************************/

/*!
    \ingroup copy

    \brief copy implementation
*/
template <class InputIter, class Size, class OutputIter>
inline std::pair<InputIter, OutputIter> copy_n(InputIter first, Size count, OutputIter result) 
{
    return implementation::copy_n(first, count, result,
        typename std::iterator_traits<InputIter>::iterator_category());
}

/*************************************************************************************************/

#ifndef ADOBE_NO_DOCUMENTATION
namespace implementation {

/*************************************************************************************************/
/*!
    \ingroup copy

    REVIST (sparent) : There is an assumption here that the difference types of the two ranges are
    the same.  We need a way to promote the smaller integral type to the larger.
*/
template <typename I, // I models RandomAccessIterator
          typename F> // F models RandomAccessIterator
inline std::pair<I, F> copy_bounded(I first, I last,
                                  F result_first, F result_last,
                                  std::random_access_iterator_tag, std::random_access_iterator_tag)
{
    return adobe::copy_n(first, std::min(last - first, result_last - result_first), result_first);
}

/*!
    \ingroup copy

    \brief copy implementation
*/
template <typename I, // I models InputIterator
          typename F> // F models ForwardIterator
inline std::pair<I, F> copy_bounded(I first, I last,
                                    F result_first, F result_last,
                                    std::input_iterator_tag, std::forward_iterator_tag)
{
    while (first != last && result_first != result_last)
    {
        *result_first = *first;
        ++first; ++result_first;
    }
    
    return std::make_pair(first, result_first);
}

/*************************************************************************************************/

} // namespace implementation
#endif

/*************************************************************************************************/
/*!
    \ingroup copy

    \brief copy implementation
*/
template <typename I, // I models InputIterator
          typename F> // F models ForwardIterator
inline std::pair<I, F> copy_bounded(I first, I last, F result_first, F result_last)
{
    return implementation::copy_bounded(first, last, result_first, result_last,
        typename std::iterator_traits<I>::iterator_category(),
        typename std::iterator_traits<F>::iterator_category());
}

/*************************************************************************************************/

/*!
    \ingroup copy
    
    \brief copy implementation
*/
template <typename I, // I models InputIterator
          typename O, // O models OutputIterator
          typename T> // T == value_type(I)
inline std::pair<I, O> copy_sentinal(I f, O o, const T& x)
{
    while (*f != x) {
        *o = *f;
        ++f, ++o;
    }
    return std::make_pair(f, o);
}

/*************************************************************************************************/

/*!
    \ingroup copy
    
    \brief copy implementation
*/
template <typename I, // I models InputIterator
          typename O> // O models OutputIterator
inline std::pair<I, O> copy_sentinal(I f, O o)
{
    return copy_sentinal(f, o, typename std::iterator_traits<I>::value_type());
}

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/

#endif

/*************************************************************************************************/
