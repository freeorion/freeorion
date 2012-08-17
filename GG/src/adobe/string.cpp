/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#include <GG/adobe/string.hpp>

#include <cstring>

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/

namespace version_1 {

string_t::string_t(const char* s)
{
        assign(s, s + std::strlen(s));
}

string_t::string_t(const char* s, std::size_t length)
{
        assign(s, s + length);
}

string_t::string_t(const std::string& s)
{
        assign(s.begin(), s.end());
}

string_t::size_type string_t::capacity() const
{
        const storage_type::size_type storage_capacity = storage_m.capacity();
        return (0 == storage_capacity ? 0 : storage_capacity - 1);
}

void string_t::push_back(value_type c)
{
        if (!empty())
                storage_m.pop_back();
        
        storage_m.push_back(c);
        storage_m.push_back(value_type(0));
}

/*************************************************************************************************/

string16_t::string16_t(const boost::uint16_t* s, std::size_t length)
{
        assign(s, s + length);
}

string16_t::string16_t(const boost::uint16_t* s)
{
        const boost::uint16_t* last(s);
        while (0 != *last) ++last;
        assign(s, last);
}

const boost::uint16_t* string16_t::c_str() const
{
        static const boost::uint16_t empty_string_s(0);
        
        return empty() ? &empty_string_s : &storage_m[0];
}

string16_t::size_type string16_t::capacity() const
{
        const storage_type::size_type storage_capacity = storage_m.capacity();
        return (0 == storage_capacity ? 0 : storage_capacity - 1);
}

void string16_t::push_back(value_type c)
{
        if (!empty())
                storage_m.pop_back();
        
        storage_m.push_back(c);
        storage_m.push_back(value_type(0));
}

void string16_t::append(const boost::uint16_t* s)
{
        const boost::uint16_t* last(s);
        while (0 != *last) ++last;
        append(s, last);
}

/*************************************************************************************************/

} // namespace version_1

/*************************************************************************************************/

#if defined(ADOBE_STD_SERIALIZATION)

std::ostream& operator << (std::ostream& os, const adobe::string_t& t)
{
                os << t.c_str();
                return os;
}

#endif

/*************************************************************************************************/

} // namespace adobe

#if 0

/*************************************************************************************************/

ADOBE_ANY_SERIALIZE(adobe::string_t, "adobe:string")

/*************************************************************************************************/

#endif
