/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_DICTIONARY_FWD_HPP
#define ADOBE_DICTIONARY_FWD_HPP

/*************************************************************************************************/

#include <GG/adobe/config.hpp>

#include <GG/adobe/any_regular_fwd.hpp>
#include <GG/adobe/closed_hash_fwd.hpp>
#include <GG/adobe/name_fwd.hpp>

/*************************************************************************************************/

namespace adobe {
namespace version_1 {

/*************************************************************************************************/

//!\ingroup abi_container
typedef closed_hash_map<name_t, any_regular_t> dictionary_t;

/*************************************************************************************************/

} // namespace version_1

using version_1::dictionary_t;

#if defined(ADOBE_NO_DOCUMENTATION)
/* REVISIT (mmarcus) : doxygen doesn't seem to understand using
declarartions.  This is a doxygen only hack to keep reference links
from breaking.
*/
//!\ingroup abi_container
typedef version1::closed_hash_map<name_t, any_regular_t> dictionary_t;
#endif

} // namespace adobe

/*************************************************************************************************/

#endif
