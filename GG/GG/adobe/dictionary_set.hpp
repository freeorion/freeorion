/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/**************************************************************************************************/

#ifndef ADOBE_DICTIONARY_SET_HPP
#define ADOBE_DICTIONARY_SET_HPP

/**************************************************************************************************/

#include <GG/adobe/config.hpp>

#include <GG/adobe/dictionary.hpp>

/**************************************************************************************************/

namespace adobe {

/**************************************************************************************************/
/*!
    Performs a set union between two dictionaries.
    
    dictionary_t is a hash-based associative container, so does not order
    elements lexicographically. This routine creates temporary table indices to
    sort the dictionary entries lexicographically by key, then performs the
    set_union.

    See Also:
        - http://www.sgi.com/tech/stl/set_union.html
*/
dictionary_t dictionary_union(const dictionary_t& src1,
                              const dictionary_t& src2);

/**************************************************************************************************/

} // namespace adobe

/**************************************************************************************************/
// ADOBE_DICTIONARY_SET_HPP
#endif

/**************************************************************************************************/
