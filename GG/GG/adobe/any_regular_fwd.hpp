/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_VALUE_FWD_HPP
#define ADOBE_VALUE_FWD_HPP

#include <GG/adobe/config.hpp>

/*************************************************************************************************/

namespace adobe {

namespace version_1 {

/*************************************************************************************************/

class any_regular_t;

/*************************************************************************************************/

} // namespace version_1

using version_1::any_regular_t;

#if defined(ADOBE_NO_DOCUMENTATION)
/* REVISIT (mmarcus) : doxygen doesn't seem to understand using
declarartions.  This is a doxygen only hack to keep reference links
from breaking.
*/
//!\ingroup abi_container
typedef version_1::any_regular_t any_regular_t;
#endif

} // namespace adobe
    
/*************************************************************************************************/

#endif
    
/*************************************************************************************************/
