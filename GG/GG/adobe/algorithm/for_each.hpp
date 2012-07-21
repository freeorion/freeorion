/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_ALGORITHM_FOR_EACH_HPP
#define ADOBE_ALGORITHM_FOR_EACH_HPP

#include <GG/adobe/config.hpp>

#include <boost/bind.hpp>
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>

#include <algorithm>

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/
/*!
\defgroup for_each for_each
\ingroup non_mutating_algorithm

\note Unlike \c std::for_each(), \c adobe::for_each() does not return a copy of \c f because \c f
may not be a function object (it must model \ref concept_convertible_to_function). If it is
necessary to retrieve information from the function object, pass a \c boost::reference_wrapper of
the function object using \c boost::ref() instead.
*/
/*************************************************************************************************/
/*!
    \ingroup for_each

    \brief for_each implementation
*/
template <class InputIterator, class UnaryFunction>
inline void for_each(InputIterator first, InputIterator last, UnaryFunction f)
{
    std::for_each(first, last, boost::bind(f, _1));
}

/*!
    \ingroup for_each

    \brief for_each implementation
*/
template <class InputRange, class UnaryFunction>
inline void for_each(InputRange& range, UnaryFunction f)
{
    adobe::for_each(boost::begin(range), boost::end(range), f);
}

/*!
    \ingroup for_each

    \brief for_each implementation
*/
template <class InputRange, class UnaryFunction>
inline void for_each(const InputRange& range, UnaryFunction f)
{
    adobe::for_each(boost::begin(range), boost::end(range), f);
}

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/

#endif

/*************************************************************************************************/
