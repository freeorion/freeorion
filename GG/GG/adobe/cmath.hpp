/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

/*

REVISIT (sparent) : Need to replicate the boost configuration tests to figure out when to fall
back to include math.h. This also needs to add any other C99 math.h extensions.

*/

#ifndef ADOBE_CMATH_HPP
#define ADOBE_CMATH_HPP

#include <GG/adobe/config.hpp>

#include <cmath>
#include <functional>

/*************************************************************************************************/

#if defined(__MWERKS__)
/*
    Any (previously) supported version of metrowerks had the C99/TR1 cmath extensions in the
    standard namespace in <cmath>.
*/
#define ADOBE_HAS_C99_STD_MATH_H

#elif defined(__GNUC__)

// Guessing at gcc 3 support
#if  (__GNUC__ == 3) && (__GNUC_MINOR__ > 2)

#define ADOBE_HAS_CPP_CMATH

#elif (__GNUC__ == 4)
/*
    The currently supported version of GNUC has C99 extensions in math.h. But no TR1 extensions.
*/
#define ADOBE_HAS_C99_MATH_H
 
#else
#error "Unknown GCC compiler configuration for cmath (last known version is 4.0.1)."
#endif

#elif defined(_MSC_VER)

/*
    The currently supported version of VC++ has no C99 extensions.
*/

#if _MSC_VER > 1500
#error "Unknown MSC compiler configureation for cmath (last knownversion is VC++ 9.0)."
#endif

#define ADOBE_HAS_CPP_CMATH

#else
#error "Unknown compiler configuration for cmath."
#endif

/*************************************************************************************************/

#if defined(ADOBE_HAS_C99_STD_MATH_H)

namespace adobe {

using std::float_t;
using std::double_t;

using std::round;
using std::lround;
using std::trunc;

} // namespace adobe

/*************************************************************************************************/

#elif   defined(ADOBE_HAS_CPP_CMATH)

namespace adobe {

typedef float   float_t;
typedef double  double_t;

/*************************************************************************************************/

inline float trunc(float x)
{ return x < 0.0f ? std::ceil(x) : std::floor(x); }

inline double trunc(double x)
{ return x < 0.0 ? std::ceil(x) : std::floor(x); }

/*************************************************************************************************/

inline float round(float x)
{ return trunc(x + (x < 0.0f ? -0.5f : 0.5f)); }

inline double round(double x)
{ return trunc(x + (x < 0.0 ? -0.5 : 0.5)); }

/*************************************************************************************************/

inline long lround(float x)
{ return static_cast<long>(x + (x < 0.0f ? -0.5f : 0.5f)); }

inline long lround(double x)
{ return static_cast<long>(x + (x < 0.0 ? -0.5 : 0.5)); }

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/

#elif defined(ADOBE_HAS_C99_MATH_H)

#include <math.h>

namespace adobe {

using ::float_t;
using ::double_t;

/*************************************************************************************************/

using ::round;
using ::lround;
using ::trunc;

inline float round(float x) { return ::roundf(x); }
inline long lround(float x) { return ::lroundf(x); }
inline float trunc(float x) { return ::truncf(x); }

/*************************************************************************************************/

} // namespace adobe

#elif defined(ADOBE_NO_DOCUMENTATION)

namespace adobe {

/*!
\name Compatibility
Compatibility features for C99/TR1 <code>\<cmath\></code>.
@{
*/

/*!
\ingroup cmath
*/

typedef Float double_t;
typedef Float float_t;

double round(double x);
float round(float x);
long lround(double x);
long lround(float x);
double trunc(double x);
float trunc(float x);
/*! @} */

} // namespace adobe

#endif

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/

template <typename A, typename R> struct nearest_cast_fn;

/*************************************************************************************************/

inline double round_half_up(double x)
{ return std::floor(x + 0.5); }

inline float round_half_up(float x)
{ return std::floor(x + 0.5f); }

inline long lround_half_up(double x)
{ return static_cast<long>(std::floor(x + 0.5)); }

inline long lround_half_up(float x)
{ return static_cast<long>(std::floor(x + 0.5f)); }

/*
    REVISIT (sparent) : Should complete the rounding modes by providing a round_half_even()
    function.
    
    Names are borrowed from the EDA rounding modes:
    
    <http://www.gobosoft.com/eiffel/gobo/math/decimal/>
*/

/*************************************************************************************************/

template <typename R, typename A>
inline R nearest_cast(const A& x)
{ return nearest_cast_fn<A, R>()(x); }

/*************************************************************************************************/

template <typename A, typename R>
struct nearest_cast_fn : std::unary_function<A, R>
{
    R operator()(const A& x) const { return static_cast<R>(round_half_up(x)); }
};

template <typename A>
struct nearest_cast_fn<A, float> : std::unary_function<A, float>
{
    float operator()(const A& x) const { return static_cast<float>(x); }
};

template <typename A>
struct nearest_cast_fn<A, double> : std::unary_function<A, double>
{
    double operator()(const A& x) const { return static_cast<double>(x); }
};

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/

#endif

/*************************************************************************************************/
