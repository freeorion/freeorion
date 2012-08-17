/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_FUNCTIONAL_OPERATOR_HPP
#define ADOBE_FUNCTIONAL_OPERATOR_HPP

#include <GG/adobe/config.hpp>

#include <functional>

#include <GG/adobe/move.hpp>

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/

/*! \addtogroup misc_functional
@{
*/
struct equal_to
{
    typedef bool result_type;

    template <typename T> // T models Regular
    bool operator()(const T& x, const T& y) const { return std::equal_to<T>()(x, y); }
};

struct not_equal_to
{
    typedef bool result_type;

    template <typename T> // T models Regular
    bool operator()(const T& x, const T& y) const { return std::not_equal_to<T>()(x, y); }
};

struct greater
{
    typedef bool result_type;

    template <typename T> // T models Regular
    bool operator()(const T& x, const T& y) const { return std::greater<T>()(x, y); }
};

struct less
{
    typedef bool result_type;

    template <typename T> // T models Regular
    bool operator()(const T& x, const T& y) const { return std::less<T>()(x, y); }
};

struct greater_equal
{
    typedef bool result_type;

    template <typename T> // T models Regular
    bool operator()(const T& x, const T& y) const { return std::greater_equal<T>()(x, y); }
};

struct less_equal
{
    typedef bool result_type;

    template <typename T> // T models Regular
    bool operator()(const T& x, const T& y) const { return std::less_equal<T>()(x, y); }
};

struct logical_and
{
    typedef bool result_type;

    template <typename T> // T models Regular
    bool operator()(const T& x, const T& y) const { return std::logical_and<T>()(x, y); }
};

struct logical_or
{
    typedef bool result_type;

    template <typename T> // T models Regular
    bool operator()(const T& x, const T& y) const { return std::logical_or<T>()(x, y); }
};

struct logical_not
{
    typedef bool result_type;

    template <typename T> // T models Regular
    bool operator()(const T& x) const { return std::logical_not<T>()(x); }
};

struct assign
{
    typedef void result_type;
    
    template <typename T> // T models Regular
    void operator()(const T& x, T& r, typename copy_sink<T>::type = 0) { r = x; }
    
    template <typename T> // T models Movable
    void operator()(T x, T& r, typename move_sink<T>::type = 0) { r = ::adobe::move(x); }
};

/*************************************************************************************************/

template <typename T> // T models Regular
struct pointer_to
{
    typedef T* result_type;

    T* operator()(T& x) const { return &x; }
};

/*************************************************************************************************/

template <typename T>
struct identity
{
    typedef T& result_type;

    T& operator()(T& x) const { return x; }
};

/*************************************************************************************************/

/*
REVISIT (sparent) : the problem noted below with boost 1.28 may have been the need for a
result_type for bind. Try this again using member templates and an explicit void result type.
*/

template <typename T>
struct delete_ptr;

/*!

\brief A function object for type \c T that invokes \c delete on a \c T*.

delete_ptr is similar to boost::lambda::delete_ptr except it inhereits from
std::unary_function so it can also be used with boost::bind.

That requirement changes the interface so the pointer type must be provided in advance. This is as 
of boost 1.28. This functionality may be included in later releases of boost.
*/

template <typename T>
struct delete_ptr<T*> : std::unary_function<T*, void>
{
    void operator () (T* x) const throw()
    { delete x; }
};

template <typename T>
struct delete_ptr<T* const> : std::unary_function<T*, void>
{
    void operator () (T* x) const throw()
    { delete x; }
};

/*!

\brief A function object for type \c T that invokes \c delete[] on an array of \c T.

delete_ptr is similar to boost::lambda::delete_ptr except it inhereits from 
std::unary_function so it can also be used with boost::bind.

That requirement changes the interface so the pointer type must be provided in advance. This is as 
of boost 1.28. This functionality may be included in later releases of boost.
*/

template <typename T>
struct delete_ptr<T(*)[]> : std::unary_function<T*, void>
{
    void operator () (T* x) const throw()
    { delete [] x; }
};

template <typename T>
struct delete_ptr<T(* const)[]> : std::unary_function<T*, void>
{
    void operator () (T* x) const throw()
    { delete [] x; }
};

/*************************************************************************************************/

template<class T>
struct constructor
{
  typedef T result_type;

  T operator()() const {
    return T();
  }

  template<class A1>
  T operator()(const A1& a1) const {
    return T(a1);
  }

  template<class A1, class A2>
  T operator()(const A1& a1, const A2& a2) const {
    return T(a1, a2);
  }

  template<class A1, class A2, class A3>
  T operator()(const A1& a1, const A2& a2, const A3& a3) const {
    return T(a1, a2, a3);
  }

  template<class A1, class A2, class A3, class A4>
  T operator()(const A1& a1, const A2& a2, const A3& a3, const A4& a4) const {
    return T(a1, a2, a3, a4);
  }

  template<class A1, class A2, class A3, class A4, class A5>
  T operator()(const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5) const {
    return T(a1, a2, a3, a4, a5);
  }
};

/*************************************************************************************************/

template <typename T> // T models Regular
struct indirect
{
    typedef T result_type;

    template <typename P> // models TrivialIterator where value_type(P) == T
    T& operator () (P x) const
        { return *x; }
};

/*************************************************************************************************/

template <class T>
struct bitwise_or
    : std::binary_function<T, T, T>
{
    T operator()(const T& x, const T& y) const {return x | y;}
};

/*************************************************************************************************/

template <class T>
struct bitwise_and
    : std::binary_function<T, T, T>
{
    T operator()(const T& x, const T& y) const {return x & y;}
};

/*************************************************************************************************/

template <class T>
struct bitwise_xor
    : std::binary_function<T, T, T>
{
    T operator()(const T& x, const T& y) const {return x ^ y;}
};

/*************************************************************************************************/

/// \brief plus function object whose arguments may be of different type.
template <typename T1, typename T2>
struct plus_asymmetric : public std::binary_function<T1,T2,T1> 
{
    T1 operator()(T1 f1, T2 f2) const {
        return f1+f2;
    }
};

/*************************************************************************************************/

/// \brief operator++ wrapped in a function object
template <typename T>
struct inc : public std::unary_function<T,T> 
{
    T operator()(T x) const { return ++x; }
};

/*************************************************************************************************/

/// \brief operator-- wrapped in a function object
template <typename T>
struct dec : public std::unary_function<T,T> 
{
    T operator()(T x) const { return --x; }
};

//!@}

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/

#endif

/*************************************************************************************************/
