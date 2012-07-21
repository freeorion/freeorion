/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_IGNORE_UNUSED_HPP
#define ADOBE_IGNORE_UNUSED_HPP

/*************************************************************************************************/

#include <GG/adobe/config.hpp>

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/

//! \ingroup testing 
template <typename T0>
inline void ignore_unused(const T0&) { }

//! \ingroup testing 
template <typename T0, typename T1>
inline void ignore_unused(const T0&, const T1&) { }

//! \ingroup testing
template <typename T0, typename T1, typename T2>
inline void ignore_unused(const T0&, const T1&, const T2&) { }

//! \ingroup testing
template <typename T0, typename T1, typename T2, typename T3>
inline void ignore_unused(const T0&, const T1&, const T2&, const T3&) { }

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/

// ADOBE_IGNORE_UNUSED_HPP
#endif

/*************************************************************************************************/
