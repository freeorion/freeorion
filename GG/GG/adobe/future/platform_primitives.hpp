/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#ifndef ADOBE_PLATFORM_PRIMITIVES_EDGE_HPP
#define ADOBE_PLATFORM_PRIMITIVES_EDGE_HPP

#include <GG/Base.h>

namespace GG {
    class Wnd;
}

/****************************************************************************************************/

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/

typedef GG::Wnd* platform_display_type;

typedef std::pair<GG::Key, boost::uint32_t> key_type;

/*************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/

#endif

/****************************************************************************************************/
