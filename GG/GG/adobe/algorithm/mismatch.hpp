/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_ALGORITHM_MISMATCH_HPP
#define ADOBE_ALGORITHM_MISMATCH_HPP

#include <GG/adobe/config.hpp>

#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <boost/bind.hpp>

#include <algorithm>


#if ADOBE_HAS_CPLUS0X_CONCEPTS

namespace std { concept_map CopyConstructible<pair<const unsigned char*, const unsigned char*> > {}}

#endif
/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/
/*!
\defgroup mismatch mismatch
\ingroup non_mutating_algorithm

\see
    - STL documentation for \ref stldoc_mismatch
*/
/*************************************************************************************************/
/*!
    \ingroup mismatch

    \brief mismatch implementation
*/
template <class InputRange1, class InputIterator2>
inline std::pair<typename boost::range_iterator<InputRange1>::type, InputIterator2>
mismatch(InputRange1& range1, InputIterator2 first2)
{
    return std::mismatch(boost::begin(range1), boost::end(range1), first2);
}


/*!
    \ingroup mismatch

    \brief mismatch implementation
*/
template <class InputRange1, class InputIterator2>
inline std::pair<typename boost::range_const_iterator<InputRange1>::type, InputIterator2>
mismatch(const InputRange1& range1, InputIterator2 first2)
{
    return std::mismatch(boost::begin(range1), boost::end(range1), first2);
}

/*!
    \ingroup mismatch

    \brief mismatch implementation
*/
template <class InputIterator1, class InputIterator2, class BinaryPredicate>
inline std::pair<InputIterator1, InputIterator2> mismatch(InputIterator1 first1,
                                                          InputIterator1 last1, 
                                                          InputIterator2 first2,
                                                          BinaryPredicate pred)
{
    return std::mismatch(first1, last1, first2, boost::bind(pred, _1, _2));
}

/*!
    \ingroup mismatch

    \brief mismatch implementation
*/
template <class InputRange1, class InputIterator2, class BinaryPredicate>
inline std::pair<typename boost::range_iterator<InputRange1>::type, InputIterator2>
mismatch(InputRange1& range1, InputIterator2 first2, BinaryPredicate pred)
{
    return adobe::mismatch(boost::begin(range1), boost::end(range1), first2, pred);
}

/*!
    \ingroup mismatch

    \brief mismatch implementation
*/
template <class InputRange1, class InputIterator2, class BinaryPredicate>
inline std::pair<typename boost::range_const_iterator<InputRange1>::type, InputIterator2>
mismatch(const InputRange1& range1, InputIterator2 first2, BinaryPredicate pred)
{
    return adobe::mismatch(boost::begin(range1), boost::end(range1), first2, pred);
}

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/

#endif

/*************************************************************************************************/
