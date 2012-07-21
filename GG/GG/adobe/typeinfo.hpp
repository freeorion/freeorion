/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/**************************************************************************************************/

#ifndef ADOBE_TYPEINFO_HPP
#define ADOBE_TYPEINFO_HPP

/**************************************************************************************************/

#include <GG/adobe/config.hpp>

#include <boost/operators.hpp>
#include <boost/range/as_literal.hpp>

#include <cstddef>
#include <functional>
#include <string>
#include <typeinfo>

#include <GG/adobe/algorithm/copy.hpp>

#ifndef NDEBUG
#include <iosfwd>
#endif

/**************************************************************************************************/

namespace adobe {

/*!
\defgroup type_info_related ABI-Safe type_info 
\ingroup abi_safe
\brief Partial re-implementation of standard type_info . As part of namespace version_1
it is guaranteed to remain binary compatible over time. It is a purely static mechanism.
In addition, classes must manually register to using the provided macros.
*/
    
/**************************************************************************************************/

namespace implementation {

/**************************************************************************************************/

struct type_instance_t
{
    const std::type_info*   type_info_m;
    const char*             name_m;
    const type_instance_t*  parameter_m[6];
    
    bool requires_std_rtti() const;
};

template <typename O> // O models OutputIterator
// requires value_type(O) == char
O serialize(const type_instance_t& x, O out)
{
        if (x.type_info_m) { return copy_sentinal(x.type_info_m->name(), out).second; }
    out = copy_sentinal(x.name_m, out).second;
    if (x.parameter_m[0]) {
        *out++ = '<'; out = serialize(*x.parameter_m[0], out);
        for (const type_instance_t* const* xp = &x.parameter_m[1]; *xp; ++xp) {
            *out++ = ','; out = serialize(**xp, out);
        }
        *out++ = '>';
    }
    return out;
};
    
bool before(const type_instance_t& x, const type_instance_t& y);
bool operator==(const type_instance_t& x, const type_instance_t& y);

inline bool operator!=(const type_instance_t& x, const type_instance_t& y) { return !(x == y); }

/**************************************************************************************************/

} // namespace implementation

/**************************************************************************************************/

namespace version_1 {


#ifndef ADOBE_REQUIRES_STD_RTTI
#define ADOBE_REQUIRES_STD_RTTI 1
#endif

#if !ADOBE_REQUIRES_STD_RTTI

template <typename T, typename Any = void>
struct make_type_info
{ };

#else

template <typename T, typename Any = void>
struct make_type_info { static const implementation::type_instance_t value; };

template <typename T, typename Any>
const implementation::type_instance_t make_type_info<T, Any>::value = { &typeid(T*) };

#endif

/**************************************************************************************************/

template <typename Any, typename T0, std::size_t Size>
struct make_type_info<T0[Size], Any>
{
    static const implementation::type_instance_t value;
    static const char name_s[256];
};

template <typename Any, typename T0, std::size_t Size>
const char make_type_info<T0[Size], Any>::name_s[256]
    = { 'a', 'r', 'r', 'a', 'y', '['
        , Size / 1000000000UL % 10 + '0'
        , Size / 100000000UL % 10 + '0'
        , Size / 10000000UL % 10 + '0'
        , Size / 1000000UL % 10 + '0'
        , Size / 100000UL % 10 + '0'
        , Size / 10000UL % 10 + '0'
        , Size / 1000UL % 10 + '0'
        , Size / 100UL % 10 + '0'
        , Size / 10UL % 10 + '0'
        , Size / 1UL % 10 + '0'
        ,']' };

template <typename Any, typename T0, std::size_t Size>
const implementation::type_instance_t make_type_info<T0[Size], Any>::value
    = { 0, &name_s[0], { &make_type_info<T0>::value} };

/**************************************************************************************************/

template <typename Any, typename T0, std::size_t Size>
struct make_type_info<const T0[Size], Any>
{
    static const implementation::type_instance_t value;
    static const char name_s[256];
};

template <typename Any, typename T0, std::size_t Size>
const char make_type_info<const T0[Size], Any>::name_s[256]
    = { 'a', 'r', 'r', 'a', 'y', '['
        , Size / 1000000000UL % 10 + '0'
        , Size / 100000000UL % 10 + '0'
        , Size / 10000000UL % 10 + '0'
        , Size / 1000000UL % 10 + '0'
        , Size / 100000UL % 10 + '0'
        , Size / 10000UL % 10 + '0'
        , Size / 1000UL % 10 + '0'
        , Size / 100UL % 10 + '0'
        , Size / 10UL % 10 + '0'
        , Size / 1UL % 10 + '0'
        ,']' };

template <typename Any, typename T0, std::size_t Size>
const implementation::type_instance_t make_type_info<const T0[Size], Any>::value
    = { 0, &name_s[0], { &make_type_info<const T0>::value} };
    
/**************************************************************************************************/

/*!
\ingroup type_info_related
\brief Partial re-implementation of standard type_info . 
*/
class type_info_t : boost::equality_comparable<type_info_t>
{
 public:

/*!
\brief Returns a null-terminated byte-string corresponding to the name of the type. 
*/
    const char* name() const
    { return identity_m->type_info_m ? identity_m->type_info_m->name() : identity_m->name_m; }
   
/*!
\brief Compares the current object with x. 
Returns true if *this precedes x in the implementationís collation order
*/
    bool before(const type_info_t& x) const
    { return adobe::implementation::before(*identity_m, *x.identity_m); }
    
    bool requires_std_rtti() const
    { return identity_m->requires_std_rtti(); }
    
/*!
\brief Compares the current object with x. 
Returns true if the two values describe the same type.
*/  
    friend bool inline operator==(const type_info_t& x, const type_info_t& y)
    { return *x.identity_m == *y.identity_m; }
    
#ifndef NDEBUG
    friend std::ostream& operator<<(std::ostream& out, const type_info_t& x);
#endif

    template <typename T> 
    friend type_info_t type_info();
    
    friend struct aggregate_type_info_t;

    template <typename O>
    friend inline O serialize(const type_info_t& x, O out)
    { return serialize(*x.identity_m, out); }
    
 private:
    explicit type_info_t(const implementation::type_instance_t* x) : identity_m(x) { }

    const implementation::type_instance_t* identity_m;
};

/*!
\ingroup type_info_related
\brief retruns the type_info_t object corresponding to the type T
*/
template <typename T>
inline type_info_t type_info()
{ return type_info_t(&make_type_info<T>::value); }

/*!
\ingroup type_info_related
\brief retruns the type_info_t object corresponding to the type T
*/
template <typename T>
inline type_info_t type_info(const T&) { return type_info<const T>(); }

/*!
\ingroup type_info_related
\brief retruns the type_info_t object corresponding to the type T
*/
template <typename T>
inline type_info_t type_info(T&) { return type_info<T>(); }
    
/**************************************************************************************************/

struct aggregate_type_info_t
{
    operator type_info_t() const { return type_info_t(&private_m); }
    
    const implementation::type_instance_t& private_m;
};

/**************************************************************************************************/

/*! 
\ingroup type_info_related
\brief Register a type for use with manual \ref adobe::version_1::type_info_t . 
This integer indicates the number of template type parameters.
For example: ADOBE_NAME_TYPE_0("string_t:version_1:adobe", adobe::version_1::string_t);
*/
#define ADOBE_NAME_TYPE_0(name, ...) \
namespace adobe { namespace version_1 { \
template <typename Any> \
struct make_type_info<__VA_ARGS__, Any> { static const implementation::type_instance_t value; }; \
template <typename Any> \
const implementation::type_instance_t make_type_info<__VA_ARGS__, Any>::value \
    = { 0, name }; \
} }

/**************************************************************************************************/

/*! 
\ingroup type_info_related
\brief Register a type for use with manual \ref adobe::version_1::type_info_t . This integer indicates the number of
template type paramters.
For example: ADOBE_NAME_TYPE_1("pointer",        T0*)
*/
#define ADOBE_NAME_TYPE_1(name, ...) \
namespace adobe { namespace version_1 { \
template <typename Any, typename T0> \
struct make_type_info<__VA_ARGS__, Any> { static const implementation::type_instance_t value; }; \
template <typename Any, typename T0> \
const implementation::type_instance_t make_type_info<__VA_ARGS__, Any>::value \
    = { 0, name, { &make_type_info<T0>::value } }; \
} }

/**************************************************************************************************/

/*! 
\ingroup type_info_related
\brief Register a type for use with manual \ref adobe::version_1::type_info_t . This integer indicates the number of
template type paramters.
ADOBE_NAME_TYPE_2("closed_hash_map:version_1:adobe",
    adobe::version_1::closed_hash_map<T0, T1, boost::hash<T0>, std::equal_to<T0> >
*/
#define ADOBE_NAME_TYPE_2(name, ...) \
namespace adobe { namespace version_1 { \
template <typename Any, typename T0, typename T1> \
struct make_type_info<__VA_ARGS__, Any> { static const implementation::type_instance_t value; }; \
template <typename Any, typename T0, typename T1> \
const implementation::type_instance_t make_type_info<__VA_ARGS__, Any>::value \
    = { 0, name, { &make_type_info<T0>::value, &make_type_info<T1>::value } }; \
} }

/**************************************************************************************************/

/*! 
\ingroup type_info_related
\brief Register a type for use with manual \ref adobe::version_1::type_info_t . This integer indicates the number of
template type paramters.
\sa ADOBE_NAME_TYPE_2
*/
#define ADOBE_NAME_TYPE_3(name, ...) \
namespace adobe { namespace version_1 { \
template <typename Any, typename T0, typename T1, typename T2> \
struct make_type_info<__VA_ARGS__, Any> { static const implementation::type_instance_t value; }; \
template <typename Any, typename T0, typename T1, typename T2> \
const implementation::type_instance_t make_type_info<__VA_ARGS__, Any>::value \
    = { 0, name, { &make_type_info<T0>::value, &make_type_info<T1>::value, \
                   &make_type_info<T2>::value} }; \
} }

/**************************************************************************************************/

/*! 
\ingroup type_info_related
\brief Register a type for use with manual \ref adobe::version_1::type_info_t . This integer indicates the number of
template type paramters.
\sa ADOBE_NAME_TYPE_2
*/
#define ADOBE_NAME_TYPE_4(name, ...) \
namespace adobe { namespace version_1 { \
template <typename Any, typename T0, typename T1, typename T2, typename T3> \
struct make_type_info<__VA_ARGS__, Any> { static const implementation::type_instance_t value; }; \
template <typename Any, typename T0, typename T1, typename T2, typename T3> \
const implementation::type_instance_t make_type_info<__VA_ARGS__, Any>::value \
    = { 0, name, { &make_type_info<T0>::value, &make_type_info<T1>::value, \
                   &make_type_info<T2>::value, &make_type_info<T3>::value} }; \
} }

/**************************************************************************************************/

/*! 
\ingroup type_info_related
\brief Register a type for use with manual \ref adobe::version_1::type_info_t . This integer indicates the number of
template type paramters.
\sa ADOBE_NAME_TYPE_2
*/
#define ADOBE_NAME_TYPE_5(name, ...) \
namespace adobe { namespace version_1 { \
template <typename Any, typename T0, typename T1, typename T2, typename T3, typename T4> \
struct make_type_info<__VA_ARGS__, Any> { static const implementation::type_instance_t value; }; \
template <typename Any, typename T0, typename T1, typename T2, typename T3, typename T4> \
const implementation::type_instance_t make_type_info<__VA_ARGS__, Any>::value \
    = { 0, name, { &make_type_info<T0>::value, &make_type_info<T1>::value, \
                   &make_type_info<T2>::value, &make_type_info<T3>::value, \
                   &make_type_info<T4>::value} }; \
} }

/**************************************************************************************************/

} // namespace version_1

using version_1::make_type_info;
using version_1::aggregate_type_info_t;
using version_1::type_info;
using version_1::type_info_t;

/**************************************************************************************************/

/*! 
\ingroup type_info_related
\brief An exception class thrown during ASL failures to cast.

<code>adobe::bad_cast</code> is a decendant of <code>std::exception</code>. 
It is intended to provide detailed type information regarding the parameters 
into a cast that failed. If no <code>typeid()</code> information was used to 
construct the object, it simply relays "bad_cast".
*/
class bad_cast : public std::bad_cast
{
 public:
    bad_cast();
    bad_cast(const std::type_info& from, const std::type_info& to);
/*!
\param from <code>typeid()</code> result for the source object.
\param to <code>typeid()</code> result for the destination type.
*/
    bad_cast(const type_info_t& from, const type_info_t& to);
    bad_cast(const bad_cast&);
    bad_cast& operator=(const bad_cast&);
    virtual ~bad_cast() throw();
/*!
\return
    Either:
        - The string "bad_cast"
        - A string detailing the source and destination types that could not be cast successfully.
*/
    virtual const char* what() const throw();

 private:
    std::string what_m;
};

/**************************************************************************************************/

// Little endian - intended to read well in the debugger.
#define ADOBE_CHAR_INT(a, b, c, d) (int(a) | (int(b) << 8) | (int(c) << 16) | (int(d) << 24))

template <typename T>
struct short_name { enum { value = ADOBE_CHAR_INT('u','n','k','n') }; };

#define ADOBE_SHORT_NAME_TYPE(a, b, c, d, T) \
namespace adobe { \
template < > struct short_name<T> { enum { value = ADOBE_CHAR_INT(a, b, c, d) }; }; \
}

/**************************************************************************************************/

} // namespace adobe

/**************************************************************************************************/

namespace std {

/*!
\ingroup type_info_related
\brief Compares the x and y. 
Returns true if x precedes y in the implementationís collation order
*/
template <>
struct less<adobe::version_1::type_info_t> :
        std::binary_function<adobe::version_1::type_info_t, adobe::version_1::type_info_t, bool>
{
    bool operator()(const adobe::version_1::type_info_t& x,
                    const adobe::version_1::type_info_t& y) const
    { return x.before(y); }
};

} // namespace std

/**************************************************************************************************/

ADOBE_NAME_TYPE_0("double",         double)
ADOBE_NAME_TYPE_0("float",          float)
ADOBE_NAME_TYPE_0("int",            int)
ADOBE_NAME_TYPE_0("short",          short)
ADOBE_NAME_TYPE_0("long",           long)
ADOBE_NAME_TYPE_0("unsigned_int",   unsigned int)
ADOBE_NAME_TYPE_0("unsigned_short", unsigned short)
ADOBE_NAME_TYPE_0("unsigned_long",  unsigned long)
ADOBE_NAME_TYPE_0("char",           char)
ADOBE_NAME_TYPE_0("signed_char",    signed char)
ADOBE_NAME_TYPE_0("unsigned_char",  unsigned char)
ADOBE_NAME_TYPE_0("bool",           bool)

ADOBE_NAME_TYPE_1("pointer",        T0*)
ADOBE_NAME_TYPE_1("const",          const T0)
ADOBE_NAME_TYPE_1("reference",      T0&)

ADOBE_SHORT_NAME_TYPE('d','b','l','e',  double)
ADOBE_SHORT_NAME_TYPE('f','l','o','t',  float)
ADOBE_SHORT_NAME_TYPE('i','n','t','_',  int)
ADOBE_SHORT_NAME_TYPE('s','h','r','t',  short)
ADOBE_SHORT_NAME_TYPE('l','o','n','g',  long)
ADOBE_SHORT_NAME_TYPE('u','i','n','t',  unsigned int)
ADOBE_SHORT_NAME_TYPE('u','s','h','r',  unsigned short)
ADOBE_SHORT_NAME_TYPE('u','l','n','g',  unsigned long)
ADOBE_SHORT_NAME_TYPE('c','h','a','r',  char)
ADOBE_SHORT_NAME_TYPE('s','c','h','r',  signed char)
ADOBE_SHORT_NAME_TYPE('u','c','h','r',  unsigned char)
ADOBE_SHORT_NAME_TYPE('b','o','o','l',  bool)

/**************************************************************************************************/

#endif

/**************************************************************************************************/
