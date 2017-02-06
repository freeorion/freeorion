/*
    Copyright 2008 T. Zachary Laine

    Use, modification and distribution are subject to the Boost Software License,
    Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt).

    See http://opensource.adobe.com/gil for most recent version including documentation.
*/

/*************************************************************************************************/

#ifndef GILEXT_TYPEDEFS_H
#define GILEXT_TYPEDEFS_H

////////////////////////////////////////////////////////////////////////////////////////
/// \file               
/// \brief Useful typedefs
/// \author Lubomir Bourdev and Hailin Jin \n
///         Adobe Systems Incorporated
/// \date 2005-2007 \n Last updated on March 8, 2006
///
////////////////////////////////////////////////////////////////////////////////////////

#include <boost/gil/typedefs.hpp>
#include "gray_alpha.hpp"

namespace boost { namespace gil {

GIL_DEFINE_BASE_TYPEDEFS(8,  gray_alpha)
GIL_DEFINE_BASE_TYPEDEFS(8s, gray_alpha)
GIL_DEFINE_BASE_TYPEDEFS(16, gray_alpha)
GIL_DEFINE_BASE_TYPEDEFS(16s,gray_alpha)
GIL_DEFINE_BASE_TYPEDEFS(32 ,gray_alpha)
GIL_DEFINE_BASE_TYPEDEFS(32s,gray_alpha)
GIL_DEFINE_BASE_TYPEDEFS(32f,gray_alpha)

} }  // namespace boost::gil

#endif
