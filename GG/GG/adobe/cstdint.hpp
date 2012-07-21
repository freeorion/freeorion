/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_CSTDINT_HPP
#define ADOBE_CSTDINT_HPP

#include <GG/adobe/config.hpp>

#include <boost/cstdint.hpp>
#include <boost/static_assert.hpp>

#if BOOST_MSVC
#include <cstddef>
#endif

/*************************************************************************************************/

namespace adobe {

/*!
\defgroup tr1 TR1 Components
\ingroup utility
@{
*/

/*************************************************************************************************/

#if defined(BOOST_HAS_STDINT_H) || defined(BOOST_MSVC)

using ::intptr_t;
using ::uintptr_t;

#else

typedef long intptr_t;
typedef unsigned long uintptr_t;

#endif

BOOST_STATIC_ASSERT(!(sizeof(intptr_t) < sizeof(void*)));
BOOST_STATIC_ASSERT(!(sizeof(uintptr_t) < sizeof(void*)));

//! @}
/*************************************************************************************************/
} // namespace adobe

/*************************************************************************************************/

#endif
