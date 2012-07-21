/*
    Copyright 2005-2008 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/
/**************************************************************************************************/

#ifndef ADOBE_ALGORITHM_CLAMP_HPP
#define ADOBE_ALGORITHM_CLAMP_HPP

#include <cassert>

#include <GG/adobe/algorithm/select.hpp>
#include <GG/adobe/functional.hpp>

namespace adobe {

/**************************************************************************************************/
/*!
\defgroup clamp clamp
\ingroup sorting

As with adobe::min and adobe::max, clamp is a select algorithm. The clamp algorithm returns min if
\c x is less than \c min and \c max if \c x is greater than max.

The clamp algorithm is stable with respect to the order <code>min, x, max</code>.

The clamp_unorderd algorithm does not presume an ordering between \c min and \c max. It is
equivalent to <code>median(min, x, max)</code>.

\see
    - \ref minmax
    - \ref median
    - \ref select
*/

/**************************************************************************************************/

/*!
    \ingroup clamp
    \brief clamp implementation
    \pre <code>!r(max, min)</code>
*/

template <typename T, typename R>
inline const T& clamp(const T& x, const T& min, const T& max, R r)
{
    return select_1_ac(min, x, max, r);
}

/*!
    \ingroup clamp
    \brief clamp implementation
    \pre <code>!(max < min)</code>
*/

template <typename T>
inline const T& clamp(const T& x, const T& min, const T& max)
{
    return select_1_ac(min, x, max, adobe::less());
}

/*!
    \ingroup clamp
    \brief clamp_unordered implementation
*/

template <typename T, typename R>
inline const T& clamp_unordered(const T& x, const T& min, const T& max, R r)
{ return select_1(min, x, max, r); }

/*!
    \ingroup clamp
    \brief clamp_unordered implementation
*/

template <typename T>
inline const T& clamp_unordered(const T& x, const T& min, const T& max)
{ return select_1(min, x, max, adobe::less()); }

/**************************************************************************************************/

} // namespace adobe

#endif
