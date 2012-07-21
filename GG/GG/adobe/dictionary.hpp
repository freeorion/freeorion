/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/**************************************************************************************************/

#ifndef ADOBE_DICTIONARY_HPP
#define ADOBE_DICTIONARY_HPP

/**************************************************************************************************/

#include <GG/adobe/config.hpp>

#include <GG/adobe/dictionary_fwd.hpp>

#include <stdexcept>

#include <GG/adobe/any_regular.hpp>
#include <GG/adobe/closed_hash.hpp>
#include <GG/adobe/name.hpp>
#include <GG/adobe/typeinfo.hpp>
#include <GG/adobe/string.hpp>

#ifdef ADOBE_STD_SERIALIZATION
#include <iosfwd>
#endif

/**************************************************************************************************/

namespace adobe {
namespace version_1 {

/**************************************************************************************************/

template <typename T>
inline bool get_value(const dictionary_t& dict, name_t key, T& value)
{
    dictionary_t::const_iterator i = dict.find(key);
    if (i == dict.end()) return false;
    return i->second.cast(value);
}

inline bool get_value(const dictionary_t& dict, name_t key, any_regular_t& value)
{
    dictionary_t::const_iterator i = dict.find(key);
    if (i == dict.end()) return false;
    value = i->second;
    return true;
}

inline const any_regular_t& get_value(const dictionary_t& dict, name_t key)
{
    dictionary_t::const_iterator i = dict.find(key);
    if (i == dict.end())
        throw std::out_of_range(make_string("dictionary_t: key '", key.c_str(), "' not found"));
        
    return i->second;
}

/**************************************************************************************************/

#ifdef ADOBE_STD_SERIALIZATION

// NOTE (sparent@adobe.com) : Code for serialization is located in source/any_regular.cpp.
std::ostream& operator<<(std::ostream& out, const dictionary_t& x);

#endif

/**************************************************************************************************/

} // namespace version_1

using version_1::get_value;

} // namespace adobe

/**************************************************************************************************/

ADOBE_SHORT_NAME_TYPE('d','i','c','t', adobe::dictionary_t)

/**************************************************************************************************/

#endif

/**************************************************************************************************/

