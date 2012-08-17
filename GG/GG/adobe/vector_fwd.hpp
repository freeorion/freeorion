/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_VECTOR_FWD_HPP
#define ADOBE_VECTOR_FWD_HPP

/*************************************************************************************************/

#include <GG/adobe/config.hpp>
#include <GG/adobe/memory_fwd.hpp>

/*************************************************************************************************/

namespace adobe {
namespace version_1 {

/*************************************************************************************************/

template <typename T, typename A = capture_allocator<T> > class vector;

/*************************************************************************************************/

} // namespace version_1

using version_1::vector;

} // namespace adobe

#endif

/*************************************************************************************************/
