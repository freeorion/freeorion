//! GiGi - A GUI for OpenGL
//!
//!  Copyright (C) 2003-2008 T. Zachary Laine <whatwasthataddress@gmail.com>
//!  Copyright (C) 2013-2020 The FreeOrion Project
//!
//! Released under the GNU Lesser General Public License 2.1 or later.
//! Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
//! SPDX-License-Identifier: LGPL-2.1-or-later

//! @file GG/StrongTypedef.h
//!
//! Contains macros used to create "strong typedefs", that is value types that
//! are not mutually interoperable with each other or with builtin types for
//! extra type safety.

#ifndef _GG_StrongTypedef_h_
#define _GG_StrongTypedef_h_


#include <iostream>
#include <type_traits>


namespace GG {

/** Overload of Value() for int. */
inline constexpr int Value(int i)
{ return i; }

/** Overload of Value() for double. */
inline constexpr double Value(double d)
{ return d; }

/** Overload of Value() for std::size_t. */
inline constexpr std::size_t Value(std::size_t s)
{ return s; }

}

#define GG_MEMBER_BOOL_OP_SELF_TYPE(op, rhs_type) \
    inline constexpr bool operator op (rhs_type rhs) const \
    { return m_value op rhs.m_value; }

#define GG_MEMBER_BOOL_OP_OTHER_TYPE(op, rhs_type) \
    inline constexpr bool operator op (rhs_type rhs) const \
    { return m_value op Value(rhs); }

#define GG_MEMBER_NEG_INCR_DECR(this_type)       \
    inline constexpr this_type operator-() const \
    { return this_type(-m_value); }              \
    inline constexpr this_type& operator++()     \
    {                                            \
        ++m_value;                               \
        return *this;                            \
    }                                            \
    inline constexpr this_type& operator--()     \
    {                                            \
        --m_value;                               \
        return *this;                            \
    }                                            \
    inline constexpr this_type operator++(int)   \
    {                                            \
        this_type retval(m_value);               \
        ++m_value;                               \
        return retval;                           \
    }                                            \
    inline constexpr this_type operator--(int)   \
    {                                            \
        this_type retval(m_value);               \
        --m_value;                               \
        return retval;                           \
    }

#define GG_MEMBER_ASSIGN_OP_SELF_TYPE(op, rhs_type)         \
    inline constexpr rhs_type& operator op (rhs_type rhs)   \
    {                                                       \
        m_value op rhs.m_value;                             \
        return *this;                                       \
    }

#define GG_MEMBER_ASSIGN_OP_OTHER_TYPE_DECL(op, self_type, rhs_type) \
    inline constexpr self_type& operator op ## = (rhs_type rhs)

#define GG_MEMBER_ASSIGN_OP_OTHER_TYPE(op, self_type, rhs_type)  \
    GG_MEMBER_ASSIGN_OP_OTHER_TYPE_DECL(op, self_type, rhs_type) \
    {                                                            \
        m_value = static_cast<self_type::value_type>(            \
            m_value op Value(rhs)                                \
        );                                                       \
        return *this;                                            \
    }

#define GG_MEMBER_OP_OTHER_TYPE_DECL(op, self_type, rhs_type) \
    inline constexpr self_type& operator op (rhs_type rhs) const

#define GG_MEMBER_OP_OTHER_TYPE(op, self_type, rhs_type)  \
    GG_MEMBER_OP_OTHER_TYPE_DECL(op, self_type, rhs_type) \
    { return self_type(m_value op Value(rhs)); }

#define GG_NONMEMBER_OP_SELF_TYPE(op, self_type)                            \
    inline constexpr self_type operator op (self_type lhs, self_type rhs)   \
    { return lhs op ## = rhs; }

#define GG_NONMEMBER_OP_OTHER_TYPE(op, self_type, rhs_type)               \
    inline constexpr self_type operator op (self_type lhs, rhs_type rhs)  \
    { return lhs op ## = Value(rhs); }

#define GG_MEMBER_SELF_COMPARATORS(self_type)    \
    GG_MEMBER_BOOL_OP_SELF_TYPE(==, self_type); \
    GG_MEMBER_BOOL_OP_SELF_TYPE(!=, self_type); \
    GG_MEMBER_BOOL_OP_SELF_TYPE(<, self_type);  \
    GG_MEMBER_BOOL_OP_SELF_TYPE(>, self_type);  \
    GG_MEMBER_BOOL_OP_SELF_TYPE(<=, self_type); \
    GG_MEMBER_BOOL_OP_SELF_TYPE(>=, self_type);

#define GG_MEMBER_OTHER_COMPARATORS(rhs_type)   \
    GG_MEMBER_BOOL_OP_OTHER_TYPE(==, rhs_type); \
    GG_MEMBER_BOOL_OP_OTHER_TYPE(!=, rhs_type); \
    GG_MEMBER_BOOL_OP_OTHER_TYPE(<, rhs_type);  \
    GG_MEMBER_BOOL_OP_OTHER_TYPE(>, rhs_type);  \
    GG_MEMBER_BOOL_OP_OTHER_TYPE(<=, rhs_type); \
    GG_MEMBER_BOOL_OP_OTHER_TYPE(>=, rhs_type);

#define GG_MEMBER_ARITH_ASSIGN_OPS_SELF_TYPE(rhs_type) \
    GG_MEMBER_ASSIGN_OP_SELF_TYPE(+=, rhs_type);       \
    GG_MEMBER_ASSIGN_OP_SELF_TYPE(-=, rhs_type);       \
    GG_MEMBER_ASSIGN_OP_SELF_TYPE(*=, rhs_type);       \
    GG_MEMBER_ASSIGN_OP_SELF_TYPE(/=, rhs_type);

#define GG_MEMBER_ARITH_ASSIGN_OPS_OTHER_TYPE(self_type, rhs_type) \
    GG_MEMBER_ASSIGN_OP_OTHER_TYPE(+, self_type, rhs_type);        \
    GG_MEMBER_ASSIGN_OP_OTHER_TYPE(-, self_type, rhs_type);        \
    GG_MEMBER_ASSIGN_OP_OTHER_TYPE(*, self_type, rhs_type);        \
    GG_MEMBER_ASSIGN_OP_OTHER_TYPE(/, self_type, rhs_type);

#define GG_NONMEMBER_ARITH_OPS_SELF_TYPE(self_type) \
    GG_NONMEMBER_OP_SELF_TYPE(+, self_type);        \
    GG_NONMEMBER_OP_SELF_TYPE(-, self_type);        \
    GG_NONMEMBER_OP_SELF_TYPE(*, self_type);        \
    GG_NONMEMBER_OP_SELF_TYPE(/, self_type);

#define GG_NONMEMBER_ARITH_OPS_OTHER_TYPE(self_type, rhs_type) \
    GG_NONMEMBER_OP_OTHER_TYPE(+, self_type, rhs_type);        \
    GG_NONMEMBER_OP_OTHER_TYPE(-, self_type, rhs_type);        \
    GG_NONMEMBER_OP_OTHER_TYPE(*, self_type, rhs_type);        \
    GG_NONMEMBER_OP_OTHER_TYPE(/, self_type, rhs_type);

#define GG_NONMEMBER_REVERSED_BOOL_OP_SET(lhs_type, self_type) \
    inline constexpr bool operator==(lhs_type x, self_type y)  \
    { return y == x; }                                         \
    inline constexpr bool operator!=(lhs_type x, self_type y)  \
    { return y != x; }                                         \
    inline constexpr bool operator<(lhs_type x, self_type y)   \
    { return !(y < x || y == x); }                             \
    inline constexpr bool operator>(lhs_type x, self_type y)   \
    { return !(y > x || y == x); }                             \
    inline constexpr bool operator<=(lhs_type x, self_type y)  \
    { return !(y < x); }                                       \
    inline constexpr bool operator>=(lhs_type x, self_type y)  \
    { return !(y > x); }

#define GG_NONMEMBER_REVERSED_ARITH_OP_SET(lhs_type, self_type)   \
    inline constexpr self_type operator+(lhs_type x, self_type y) \
    { return y += x; }                                            \
    inline constexpr self_type operator-(lhs_type x, self_type y) \
    { return -(y -= x); }                                         \
    inline constexpr self_type operator*(lhs_type x, self_type y) \
    { return y *= x; }


#define GG_STRONG_DOUBLE_TYPEDEF(name, type)                            \
    class name;                                                         \
    class name ## _d;                                                   \
    constexpr type Value(name x) noexcept;                              \
    constexpr double Value(name ## _d x) noexcept;                      \
                                                                        \
    class name ## _d                                                    \
    {                                                                   \
    private:                                                            \
        struct ConvertibleToBoolDummy {int _;};                         \
                                                                        \
    public:                                                             \
        typedef double value_type;                                      \
                                                                        \
        constexpr name ## _d() = default;                               \
        explicit constexpr name ## _d(double t) : m_value(t) {}         \
                                                                        \
        GG_MEMBER_SELF_COMPARATORS(name ## _d);                         \
                                                                        \
        GG_MEMBER_OTHER_COMPARATORS(double);                            \
                                                                        \
        operator int ConvertibleToBoolDummy::* () const                 \
        { return m_value ? &ConvertibleToBoolDummy::_ : 0; }            \
                                                                        \
        GG_MEMBER_NEG_INCR_DECR(name ## _d);                            \
                                                                        \
        GG_MEMBER_ARITH_ASSIGN_OPS_SELF_TYPE(name ## _d);               \
                                                                        \
        GG_MEMBER_ARITH_ASSIGN_OPS_OTHER_TYPE(name ## _d, double);      \
                                                                        \
        GG_MEMBER_ASSIGN_OP_OTHER_TYPE_DECL(+, name ## _d, name);       \
        GG_MEMBER_ASSIGN_OP_OTHER_TYPE_DECL(-, name ## _d, name);       \
        GG_MEMBER_ASSIGN_OP_OTHER_TYPE_DECL(*, name ## _d, name);       \
        GG_MEMBER_ASSIGN_OP_OTHER_TYPE_DECL(/, name ## _d, name);       \
                                                                        \
    private:                                                            \
        double m_value = 0.0;                                           \
                                                                        \
        friend constexpr double Value(name ## _d x) noexcept;           \
    };                                                                  \
                                                                        \
    GG_NONMEMBER_ARITH_OPS_SELF_TYPE(name ## _d);                       \
                                                                        \
    GG_NONMEMBER_ARITH_OPS_OTHER_TYPE(name ## _d, double);              \
                                                                        \
    GG_NONMEMBER_REVERSED_BOOL_OP_SET(double, name ## _d);              \
                                                                        \
    GG_NONMEMBER_REVERSED_ARITH_OP_SET(double, name ## _d);             \
                                                                        \
    inline constexpr double Value(name ## _d x) noexcept                \
    { return x.m_value; }                                               \
                                                                        \
    inline std::ostream& operator<<(std::ostream& os, name ## _d x)     \
    { os << Value(x); return os; }                                      \
                                                                        \
    inline std::istream& operator>>(std::istream& os, name ## _d& x)    \
    {                                                                   \
        double t;                                                       \
        os >> t;                                                        \
        x = name ## _d(t);                                              \
        return os;                                                      \
    }                                                                   \
                                                                        \
    void dummy_function_to_force_semicolon()

/** Creates a new type \a name, based on underlying type \a type, which is not
    interconvertible with any other numeric type.  \a type must be an integral
    type.  The resulting type has most of the operations of the underlying
    integral type.  Specifically, the type is totally ordered, incrementable,
    decrementable, and arithmetic.  The type is also interarithemtic with and
    comparable to objects of types \a type and double.  Note that the free
    functions accepting doubles return GG_STRONG_DOUBLE_TYPEDEF's called \a
    name_d.  This allows \a name objects to be used in floating point math. */
#define GG_STRONG_INTEGRAL_TYPEDEF(name, type)                          \
    GG_STRONG_DOUBLE_TYPEDEF(name, type);                               \
                                                                        \
    constexpr type Value(name x) noexcept;                              \
                                                                        \
    class name                                                          \
    {                                                                   \
    private:                                                            \
        struct ConvertibleToBoolDummy {int _;};                         \
                                                                        \
    public:                                                             \
        static_assert((std::is_integral_v<type>),                       \
            "Creating an strong integral without passing an integral type"); \
                                                                        \
        typedef type value_type;                                        \
                                                                        \
        constexpr name() = default;                                     \
        explicit constexpr name(type t) : m_value(t) {}                 \
        explicit constexpr name(name ## _d t) :                         \
            m_value(static_cast<type>(Value(t)))                        \
        {}                                                              \
                                                                        \
        name& operator=(name ## _d t)                                   \
        { m_value = static_cast<type>(Value(t)); return *this; }        \
                                                                        \
        GG_MEMBER_SELF_COMPARATORS(name);                               \
                                                                        \
        GG_MEMBER_OTHER_COMPARATORS(type);                              \
        GG_MEMBER_OTHER_COMPARATORS(name ## _d);                        \
        GG_MEMBER_OTHER_COMPARATORS(double);                            \
                                                                        \
        constexpr operator int ConvertibleToBoolDummy::* () const       \
        { return m_value ? &ConvertibleToBoolDummy::_ : 0; }            \
                                                                        \
        GG_MEMBER_NEG_INCR_DECR(name);                                  \
                                                                        \
        GG_MEMBER_ARITH_ASSIGN_OPS_SELF_TYPE(name);                     \
        GG_MEMBER_ASSIGN_OP_SELF_TYPE(%=, name);                        \
                                                                        \
        GG_MEMBER_ARITH_ASSIGN_OPS_OTHER_TYPE(name, type);              \
        GG_MEMBER_ASSIGN_OP_OTHER_TYPE(%, name, type);                  \
                                                                        \
        GG_MEMBER_ARITH_ASSIGN_OPS_OTHER_TYPE(name, name ## _d);        \
        GG_MEMBER_ARITH_ASSIGN_OPS_OTHER_TYPE(name, double);            \
                                                                        \
    private:                                                            \
        type m_value = 0;                                               \
                                                                        \
        friend class name ## _d;                                        \
        friend constexpr type Value(name x) noexcept;                   \
    };                                                                  \
                                                                        \
    GG_NONMEMBER_ARITH_OPS_SELF_TYPE(name);                             \
    GG_NONMEMBER_OP_SELF_TYPE(%, name);                                 \
                                                                        \
    GG_NONMEMBER_ARITH_OPS_OTHER_TYPE(name, type);                      \
    GG_NONMEMBER_OP_OTHER_TYPE(%, name, type);                          \
                                                                        \
    GG_NONMEMBER_REVERSED_BOOL_OP_SET(type, name);                      \
    GG_NONMEMBER_REVERSED_BOOL_OP_SET(name ## _d, name);                \
    GG_NONMEMBER_REVERSED_BOOL_OP_SET(double, name);                    \
                                                                        \
    GG_NONMEMBER_REVERSED_ARITH_OP_SET(type, name);                     \
                                                                        \
    inline constexpr name ## _d operator+(name x, double y)             \
    { return name ## _d(Value(x)) + y; }                                \
    inline constexpr name ## _d operator-(name x, double y)             \
    { return name ## _d(Value(x)) - y; }                                \
    inline constexpr name ## _d operator*(name x, double y)             \
    { return name ## _d(Value(x)) * y; }                                \
    inline constexpr name ## _d operator/(name x, double y)             \
    { return name ## _d(Value(x)) / y; }                                \
                                                                        \
    inline constexpr name ## _d operator+(double x, name y)             \
    { return x + name ## _d(Value(y)); }                                \
    inline constexpr name ## _d operator-(double x, name y)             \
    { return x - name ## _d(Value(y)); }                                \
    inline constexpr name ## _d operator*(double x, name y)             \
    { return x * name ## _d(Value(y)); }                                \
                                                                        \
    inline constexpr type Value(name x) noexcept                        \
    { return x.m_value; }                                               \
                                                                        \
    inline std::ostream& operator<<(std::ostream& os, name x)           \
    { os << Value(x); return os; }                                      \
                                                                        \
    inline std::istream& operator>>(std::istream& os, name& x)          \
    {                                                                   \
        type t;                                                         \
        os >> t;                                                        \
        x = name(t);                                                    \
        return os;                                                      \
    }                                                                   \
                                                                        \
    inline constexpr name ## _d& name ## _d::operator+=(name rhs)       \
    { m_value += Value(rhs); return *this; }                            \
    inline constexpr name ## _d& name ## _d::operator-=(name rhs)       \
    { m_value -= Value(rhs); return *this; }                            \
    inline constexpr name ## _d& name ## _d::operator*=(name rhs)       \
    { m_value *= Value(rhs); return *this; }                            \
    inline constexpr name ## _d& name ## _d::operator/=(name rhs)       \
    { m_value /= Value(rhs); return *this; }                            \
                                                                        \
    GG_NONMEMBER_ARITH_OPS_OTHER_TYPE(name ## _d, name);                \
                                                                        \
    GG_NONMEMBER_REVERSED_ARITH_OP_SET(name, name ## _d);               \
    inline constexpr name ## _d operator/(name x, name ## _d y)         \
    { return name ## _d(Value(x) / Value(y)); }                         \
                                                                        \
    void dummy_function_to_force_semicolon()

/** Creates a new type \a name, based on underlying type std::size_t, which is
    not interconvertible with any other numeric type.  The resulting type has
    most of the operations of std::size_t.  Specifically, the type is totally
    ordered, incrementable, decrementable, and arithmetic.  The type is also
    interarithemtic with and comparable to objects of type std::size_t. */
#define GG_STRONG_SIZE_TYPEDEF(name)                                    \
    class name;                                                         \
    constexpr std::size_t Value(name x) noexcept;                       \
                                                                        \
    class name                                                          \
    {                                                                   \
    private:                                                            \
        struct ConvertibleToBoolDummy {int _;};                         \
                                                                        \
    public:                                                             \
        typedef std::size_t value_type;                                 \
                                                                        \
        constexpr name() = default;                                     \
        explicit constexpr name(std::size_t t) : m_value(t) {}          \
                                                                        \
        GG_MEMBER_SELF_COMPARATORS(name);                               \
                                                                        \
        GG_MEMBER_OTHER_COMPARATORS(std::size_t);                       \
                                                                        \
        constexpr operator int ConvertibleToBoolDummy::* () const       \
        { return m_value ? &ConvertibleToBoolDummy::_ : 0; }            \
                                                                        \
        GG_MEMBER_NEG_INCR_DECR(name);                                  \
                                                                        \
        GG_MEMBER_ARITH_ASSIGN_OPS_SELF_TYPE(name);                     \
        GG_MEMBER_ASSIGN_OP_SELF_TYPE(%=, name);                        \
                                                                        \
        GG_MEMBER_ARITH_ASSIGN_OPS_OTHER_TYPE(name, std::size_t);       \
        GG_MEMBER_ASSIGN_OP_OTHER_TYPE(%, name, std::size_t);           \
                                                                        \
    private:                                                            \
        std::size_t m_value = 0;                                        \
                                                                        \
        friend class name ## _d;                                        \
        friend constexpr std::size_t Value(name x) noexcept;            \
    };                                                                  \
                                                                        \
    GG_NONMEMBER_ARITH_OPS_SELF_TYPE(name);                             \
    GG_NONMEMBER_OP_SELF_TYPE(%, name);                                 \
                                                                        \
    GG_NONMEMBER_ARITH_OPS_OTHER_TYPE(name, std::size_t);               \
    GG_NONMEMBER_OP_OTHER_TYPE(%, name, std::size_t);                   \
                                                                        \
    GG_NONMEMBER_REVERSED_BOOL_OP_SET(std::size_t, name);               \
                                                                        \
    GG_NONMEMBER_REVERSED_ARITH_OP_SET(std::size_t, name);              \
                                                                        \
    inline constexpr std::size_t Value(name x) noexcept                 \
    { return x.m_value; }                                               \
                                                                        \
    inline std::ostream& operator<<(std::ostream& os, name x)           \
    { os << Value(x); return os; }                                      \
                                                                        \
    inline std::istream& operator>>(std::istream& os, name& x)          \
    {                                                                   \
        std::size_t t;                                                  \
        os >> t;                                                        \
        x = name(t);                                                    \
        return os;                                                      \
    }                                                                   \
                                                                        \
    void dummy_function_to_force_semicolon()


#endif
