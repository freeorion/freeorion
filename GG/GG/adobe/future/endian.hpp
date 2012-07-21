/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#ifndef ADOBE_ENDIAN_HPP
#define ADOBE_ENDIAN_HPP

/****************************************************************************************************/

#include <GG/adobe/config.hpp>

#include <boost/detail/endian.hpp>

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/

struct endian
{
    enum type
    {
        big,
        little,
#if defined(BOOST_BIG_ENDIAN) || defined(__BIG_ENDIAN__)
        platform = big,
#elif defined(BOOST_LITTLE_ENDIAN) || defined(__LITTLE_ENDIAN__)
        platform = little,
#endif
    };

    enum platform_test
    {
        is_big    = platform == big,
        is_little = platform == little
    };
};

/*************************************************************************************************/

namespace implementation {

/*************************************************************************************************/

template <bool NeedsIt>
struct byteswap
{
    template <typename T>
    void operator()(T& x) const
    {
        enum
        {
            size = sizeof(T),
            end = size >> 1
        };

        for (std::size_t i(0); i < end; ++i)
            std::swap(reinterpret_cast<char*>(&x)[i],
                      reinterpret_cast<char*>(&x)[size - i - 1]);
    }
};

template <>
struct byteswap<false>
{
    template <typename T>
    void operator()(T&) const
    { }
};

/*************************************************************************************************/

} // namespace implementation

/*************************************************************************************************/

template <typename T>
inline void byteswap(T& x)
{ implementation::byteswap<true>()(x); }

/*************************************************************************************************/

template <endian::type SourceEndian>
struct endian_swap
{
    enum { need_swap = SourceEndian != endian::platform };

    template <typename T>
    void operator()(T& x) const
    { implementation::byteswap<need_swap>()(x); }
};

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/

#endif

/****************************************************************************************************/
