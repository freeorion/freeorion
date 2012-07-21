/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_NAME_HPP
#define ADOBE_NAME_HPP

#include <GG/adobe/config.hpp>

#include <GG/adobe/name_fwd.hpp>

#include <boost/utility.hpp>

#include <GG/adobe/conversion.hpp>
#include <GG/adobe/cstring.hpp>
#include <GG/adobe/typeinfo.hpp>

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/

namespace version_1 {

/*! \addtogroup abi_string
@{
*/

/*************************************************************************************************/

#if !defined(ADOBE_NO_DOCUMENTATION)

inline name_t::operator bool() const { return *name_m != 0; }

inline bool name_t::operator!() const { return !(*name_m); }

#endif

/*************************************************************************************************/

inline bool operator<(const name_t& x, const name_t& y)
{
    return adobe::strcmp(x.c_str(), y.c_str()) < 0;
}

inline bool operator == (const name_t& x, const name_t& y)
{
    /*
        The test case for equal strings is "optimized" because names are stored in hash tables and
        will often match on a find because the compiler will pool string constants.
    */
    if (x.c_str() == y.c_str()) return true;
    return adobe::strcmp(x.c_str(), y.c_str()) == 0;
}

/*************************************************************************************************/

inline const char* name_t::c_str() const
{
    return name_m;
}
    
/*************************************************************************************************/

class static_name_t : public name_t
{
 public:
    explicit static_name_t (const char* string_name = "") :
        name_t(string_name, dont_copy_t()) { }
};

struct aggregate_name_t
{
    const char* const name_m;
    operator name_t() const { return name_t(name_m, name_t::dont_copy_t()); }
};

/*
    NOTE (sparent) : This is to allow for boost::hash<> to work with name_t. boost::hash<> relies
    on argument dependent lookup.
*/

inline std::size_t hash_value(name_t name)
{
    std::size_t seed = 0; 
    for (const char* first = name.c_str(); *first; ++first) {
        seed = 5 * seed + *first;
    }
    return seed;
}
    
//!@}

/*************************************************************************************************/

} // namespace version_1
    
/*************************************************************************************************/

template <> struct promote<static_name_t> { typedef name_t type; };
template <> struct promote<aggregate_name_t> { typedef name_t type; };
    
/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/

ADOBE_NAME_TYPE_0("name_t:version_1:adobe", adobe::version_1::name_t)
ADOBE_SHORT_NAME_TYPE('n','a','m','e', adobe::version_1::name_t)

#endif

/*************************************************************************************************/
