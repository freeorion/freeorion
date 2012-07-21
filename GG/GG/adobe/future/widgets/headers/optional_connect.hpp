/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_OPTIONAL_CONNECTION_HPP
#define ADOBE_OPTIONAL_CONNECTION_HPP

/*************************************************************************************************/

#include <GG/adobe/config.hpp>

#include <boost/utility/enable_if.hpp>

/*************************************************************************************************/

#ifdef BOOST_MSVC
    #define ADOBE_OPTIONAL_CONNECT_TEST(p) &p != 0
#else
    #define ADOBE_OPTIONAL_CONNECT_TEST(p) sizeof(p) != 0
#endif

/*************************************************************************************************/

#define ADOBE_OPTIONAL_CONNECT1(ns, func)                                               \
namespace adobe {                                                                       \
namespace ns {                                                                          \
namespace implementation {                                                              \
    template <class R, class T>                                                         \
    R func(T& t,                                                                        \
    typename boost::enable_if_c<ADOBE_OPTIONAL_CONNECT_TEST(T::##func)>::type* = 0)     \
        { return t.##func(); }                                                          \
    template <class R, class T>                                                         \
    R func(volatile T&, ...)                                                            \
        { return R(); }                                                                 \
}                                                                                       \
template <class R, class T>                                                             \
R func(T& t)                                                                            \
    { return implementation::##func##<R>(t); }                                          \
} }

/*************************************************************************************************/

#define ADOBE_OPTIONAL_CONNECT2(ns, func)                                                   \
namespace adobe {                                                                           \
namespace ns {                                                                              \
namespace implementation {                                                                  \
    template <class R, class T, class U>                                                    \
    R func(T& t, U& u,                                                                      \
        typename boost::enable_if_c<ADOBE_OPTIONAL_CONNECT_TEST(T::##func)>::type* = 0)     \
        { return t.##func##(u); }                                                           \
    template <class R, class T, class U>                                                    \
    R func(volatile T&, volatile U&, ...)                                                   \
        { return R(); }                                                                     \
}                                                                                           \
template <class R, class T, class U>                                                        \
R func(T& t, U& u)                                                                          \
    { return implementation::##func##<R>(t, u); }                                           \
} }

/*************************************************************************************************/

#define ADOBE_OPTIONAL_CONNECT3(ns, func)                                                       \
namespace adobe {                                                                               \
namespace ns {                                                                                  \
namespace implementation {                                                                      \
    template <class R, class T, class U, class V>                                               \
    R func(T& t, U& u, V& v,                                                                    \
        typename boost::enable_if_c<ADOBE_OPTIONAL_CONNECT_TEST(T::##func)>::type* = 0)         \
        { return t.##func(u, v); }                                                              \
    template <class R, class T, class U, class V>                                               \
    R func(volatile T&, volatile U&, volatile V&, ...)                                          \
        { return R(); }                                                                         \
}                                                                                               \
template <class R, class T, class U, class V>                                                   \
R func(T& t, U& u, V& v)                                                                        \
    { return implementation::##func##<R>(t, u, v); }                                            \
} }

/*************************************************************************************************/

#endif // ADOBE_OPTIONAL_CONNECTION_HPP

/*************************************************************************************************/
