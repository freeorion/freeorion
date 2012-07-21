/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_CLOSED_HASH_FWD_HPP
#define ADOBE_CLOSED_HASH_FWD_HPP

/*************************************************************************************************/

#include <GG/adobe/config.hpp>

#include <functional>

#include <boost/functional/hash.hpp>

#include <GG/adobe/functional.hpp>
#include <GG/adobe/memory_fwd.hpp>
#include <GG/adobe/utility.hpp>

/*************************************************************************************************/

namespace adobe {
namespace version_1 {

/*************************************************************************************************/

template<   typename T,
            typename KeyTransform = identity<const T>,
            typename Hash = boost::hash<T>,
            typename Pred = std::equal_to<T>,
            typename A = capture_allocator<T> >
class closed_hash_set;

template<typename Key,
         typename T,
         typename Hash = boost::hash<Key>,
         typename Pred = std::equal_to<Key>,
         typename A = capture_allocator<pair<Key, T> > >
class closed_hash_map;

/*************************************************************************************************/

} // namespace version_1

using version_1::closed_hash_set;
using version_1::closed_hash_map;

#if defined(ADOBE_NO_DOCUMENTATION)
/* REVISIT (mmarcus) : doxygen doesn't seem to understand using
declarartions.  This is a doxygen only hack to keep reference links
from breaking.
*/
//!\ingroup abi_container
typedef version_1::closed_hash_set closed_hash_set;
typedef version_1::closed_hash_map closed_hash_map;
#endif

} // namespace adobe

/*************************************************************************************************/

#endif

/*************************************************************************************************/
