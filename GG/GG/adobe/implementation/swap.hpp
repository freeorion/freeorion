/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_SWAP_HPP
#define ADOBE_SWAP_HPP

/*************************************************************************************************/

#include <GG/adobe/config.hpp>

#include <algorithm>

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/
/*!
\defgroup swap swap
\ingroup algorithm

We pull swap in from \c std, with a using std::swap declaration,
and employ the idiom that all code in namespace adobe that calls swap
should call it unqualified. This is less cluttered than employing local using declaration and is
forward compatible with C++ '0x constrained swap. We recommend this idiom to clients.
*/
/*************************************************************************************************/
/*!
\ingroup swap


*/
using std::swap;

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/

#endif

/*************************************************************************************************/
