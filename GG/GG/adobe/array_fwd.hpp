/*
    Copyright 2005-2008 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_ARRAY_FWD_HPP
#define ADOBE_ARRAY_FWD_HPP

#include <GG/adobe/config.hpp>

#include <GG/adobe/any_regular_fwd.hpp>
#include <GG/adobe/vector_fwd.hpp>

/*************************************************************************************************/

namespace adobe {
namespace version_1 {

/*************************************************************************************************/

//!\ingroup abi_container
typedef vector<any_regular_t> array_t;

/*************************************************************************************************/

} // namespace version_1

using version_1::array_t;

#if defined(ADOBE_NO_DOCUMENTATION)
/* REVISIT (mmarcus) : doxygen doesn't seem to understand using
declarartions.  This is a doxygen only hack to make the typedef appear
in the documentation
*/
//!\ingroup abi_container
typedef version_1::vector<any_regular_t> array_t;
#endif

} // namespace adobe

/*************************************************************************************************/

#endif
