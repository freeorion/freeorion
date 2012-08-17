/*
    Copyright 2005-2008 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/
/**************************************************************************************************/

#ifndef ADOBE_ALGORITHM_SELECT_HPP
#define ADOBE_ALGORITHM_SELECT_HPP

#include <GG/adobe/algorithm/minmax.hpp>

namespace adobe {

/**************************************************************************************************/
/*!
\defgroup select select
\ingroup sorting

The select() algorithms are building blocks for other algorithms such as median() and
clamp(). The general form of a select algorithm is:

select_<argument_index>_<argument_ordering>()

For example: <code>select_1_ac(a, b, c)</code> means "select the second element (index starts at
zero) assuming that arguments \c a and \c b are supplied in non-decreasing order."

Althought we don't provide select functions with two arguments, adobe::min is select_0 and
adobe::max is select_1.

All of the select function are stable. An algorithm is stable if it respects the original order of
equivalent objects. So if we think of minimum and maximum as selecting, respectively, the smallest
and second-smallest from a list of two arguments, stability requires that when called with 
equivalent elements, minimum should return the first and maximum the second.

\see
    - \ref minmax
    - \ref median
    - \ref clamp
*/

/**************************************************************************************************/
/*!
    \ingroup select
    \brief select_1_ac implementation
*/
template <typename T, typename R>
inline const T& select_1_ac(const T& a, const T& b, const T& c, R r)
{
    assert(!r(c, a) && "WARNING (sparent) : a and c must be non-decreasing");
    return r(b, a) ? a : (adobe::min)(b, c, r);
}

/*!
    \ingroup select
    \brief select_1_ac implementation
*/
template <typename T, typename R>
inline T& select_1_ac(T& a, T& b, T& c, R r)
{
    assert(!r(c, a) && "WARNING (sparent) : a and c must be non-decreasing");
    return r(b, a) ? a : (adobe::min)(b, c, r);
}

/*!
    \ingroup select
    \brief select_1_ab implementation
*/
template <typename T, typename R>
inline const T& select_1_ab(const T& a, const T& b, const T& c, R r)
{
    assert(!r(b, a) && "WARNING (sparent) : a and b must be non-decreasing");
    return r(c, b) ? (adobe::max)(a, c, r) : b;
}

/*!
    \ingroup select
    \brief select_1_ab implementation
*/
template <typename T, typename R>
inline T& select_1_ab(T& a, T& b, T& c, R r)
{
    assert(!r(b, a) && "WARNING (sparent) : a and b must be non-decreasing");
    return r(c, b) ? (adobe::max)(a, c, r) : b;
}

/*!
    \ingroup select
    \brief select_1 implementation
*/
template <typename T, typename R>
inline const T& select_1(const T& a, const T& b, const T& c, R r)
{ return r(b, a) ? select_1_ab(b, a, c, r) : select_1_ab(a, b, c, r); }

/*!
    \ingroup select
    \brief select_1 implementation
*/
template <typename T, typename R>
inline T& select_1(T& a, T& b, T& c, R r)
{ return r(b, a) ? select_1_ab(b, a, c, r) : select_1_ab(a, b, c, r); }

/**************************************************************************************************/

} // namespace adobe

#endif
