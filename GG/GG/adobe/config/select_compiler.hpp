/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_CONFIG_SELECT_COMPILER_HPP
#define ADOBE_CONFIG_SELECT_COMPILER_HPP

/*************************************************************************************************/

#if defined __GNUC__
    //  GNU C++: (including Darwin)
    #include <GG/adobe/config/compiler/gcc.hpp>

#elif defined _MSC_VER
    //  Must remain the last #elif since some other vendors (Metrowerks, for
    //  example) also #define _MSC_VER
    #include <GG/adobe/config/compiler/visualc.hpp>

#endif

/*************************************************************************************************/

#endif

/*************************************************************************************************/
