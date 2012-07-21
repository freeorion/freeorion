/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#include <GG/adobe/name.hpp>
#include <GG/adobe/once.hpp>
#include <GG/adobe/implementation/string_pool.hpp>

#if defined(ADOBE_STD_SERIALIZATION)
#include <ostream>
#endif

/*************************************************************************************************/

ADOBE_GLOBAL_MUTEX_DEFINITION(name_t)

/*************************************************************************************************/

namespace {

/*************************************************************************************************/

// Precondition: length only need be non-zero if not copying
// Precondition: if string_name is null - length must be zero
const char* unique_string(const char* string_name)
{
    static const char* empty_string_s = "";

    if (!string_name || !*string_name) return empty_string_s;
    
    ADOBE_GLOBAL_MUTEX_INSTANCE(name_t);

    static adobe::unique_string_pool_t unique_string_s;

    return unique_string_s.add(string_name);
}

/*************************************************************************************************/

} // namespace

/*************************************************************************************************/

namespace adobe {
namespace version_1 {

/*************************************************************************************************/

name_t::name_t (const char* x)
{
    name_m = unique_string(x);
}

/*************************************************************************************************/

#if defined(ADOBE_STD_SERIALIZATION)

std::ostream& operator << (std::ostream& os, const adobe::name_t& t)
{
    os << t.c_str();
    return os;
}

#endif

/*************************************************************************************************/

} // namespace version_1
} // namespace adobe

/*************************************************************************************************/
