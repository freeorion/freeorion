/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_EXPRESSION_FILTER_HPP
#define ADOBE_EXPRESSION_FILTER_HPP

#include <GG/adobe/config.hpp>

#include <boost/cstdint.hpp>

#include <GG/adobe/string.hpp>
#include <GG/adobe/array.hpp>

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/

#if 0 // TZL
/*!
    Takes a unicode code point and returns the equivalent XML entity name.

    Throws if the code point does not have an XML entity mapping.

    Note: The entity name does not contain the standard
          entity prefix ('&') or suffix (';')
*/
const adobe::string_t& entity_map_find(boost::uint32_t code_point);

/*!
    Takes an XML entity name and returns the equivalent unicode code point.

    Throws if the XML entity name is invalid.

    Note: The entity name must not contain the standard
          entity prefix ('&') or suffix (';')
*/
boost::uint32_t entity_map_find(const adobe::string_t& entity);
#endif

/*!
    Returns whether or not a string has at least one character in it
    that would be escaped by entity_escape; can be faster than
    escape-and-compare, as it could return true before the entire string
    is scanned.
*/
bool needs_entity_escape(const string_t& value);

/*!
    Takes a string and converts certain ascii characters found within to
    their xml entity equivalents, returning the "entity formatted" string.
    "certain ascii characters" are defined as:
        - apostrophe (') or quote (")
        - characters with an xml entity mapping
        - non-printable ASCII characters
        - characters with the high-bit set
*/
string_t entity_escape(const string_t& value);

/*!
    Returns whether or not a string has at least one entity in it that
    would be unescaped by entity_unescape; can be faster than
    unescape-and-compare, as it could return true before the entire
    string is scanned.
*/
bool needs_entity_unescape(const string_t& value);

/*!
    Takes a string and converts the xml entity declarations found within to
    their ASCII equivalents, returning the "entity flattened" string.
*/
string_t entity_unescape(const string_t& value);

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/
// ADOBE_EXPRESSION_FILTER_HPP
#endif

/*************************************************************************************************/
