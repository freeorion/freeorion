// -*- C++ -*-
#ifndef _Random_h_
#define _Random_h_

#if defined(_MSC_VER)
  // HACK! this keeps VC 7.x from barfing when it sees "typedef __int64 int64_t;"
  // in boost/cstdint.h when compiling under windows
#  if defined(int64_t)
#    undef int64_t
#  endif
#elif defined(WIN32)
  // HACK! this keeps gcc 3.x from barfing when it sees "typedef long long uint64_t;"
  // in boost/cstdint.h when compiling under windows
#  define BOOST_MSVC -1
#endif
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_smallint.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/uniform_01.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/random/normal_distribution.hpp>
#include <ctime>
#include <sstream>
/* The interface of boost.random has changed in Version 1.31, since
 * we require 1.30+ anyway, we just check for 1.31 and up and assume
 * version 1.30 otherwise
 */
#include <boost/version.hpp>
#if BOOST_VERSION >= 103100
#  include <boost/random/variate_generator.hpp>
#endif

/** \file Random.h
    A collection of robust and portable random number generation functors and functions.
    If you need to generate one or two random numbers, prefer the functions (e.g. RandomInt()).
    If you need to generate a large volume of random numbers with the
    same parameterization,
    generate a functor (e.g. with a call to IntDist()) and then call the functor repeatedly to
    generate the numbers.  This eliminates the overhead associated with repeatedly contructing 
    distributions, when you call the Random*() functions. */

typedef boost::mt19937                            GeneratorType;

#if BOOST_VERSION < 103100
  typedef boost::uniform_smallint<GeneratorType>    SmallIntDistType;
  typedef boost::uniform_int<GeneratorType>         IntDistType;
  typedef boost::uniform_real<GeneratorType>        DoubleDistType;
  typedef boost::normal_distribution<GeneratorType> GaussianDistType;
#else
   typedef boost::variate_generator<GeneratorType,boost::uniform_smallint<> >   SmallIntDistType;
  typedef boost::variate_generator<GeneratorType,boost::uniform_int<> >         IntDistType;
  typedef boost::variate_generator<GeneratorType,boost::uniform_real<> >        DoubleDistType;
  typedef boost::variate_generator<GeneratorType,boost::normal_distribution<> > GaussianDistType;
#endif

namespace {
    GeneratorType gen; // the one random number generator driving the distributions below
    // The interface of uniform_01 didn't change.
    boost::uniform_01<GeneratorType> zero_to_one_gen(gen);
}

/** seeds the underlying random number generator used to drive all random number distributions */
inline void Seed(unsigned int seed) {gen.seed(static_cast<boost::mt19937::result_type>(seed));}

/** seeds the underlying random number generator used to drive all random number distributions with 
    the current clock time */
inline void ClockSeed() {gen.seed(static_cast<boost::mt19937::result_type>(std::time(0)));}

/** returns a functor that provides a uniform distribution of small
    integers in the range [\a min, \a max]; if the integers desired
    are larger than 10000, use IntDist() instead */
#if BOOST_VERSION < 103100
  inline SmallIntDistType SmallIntDist(int min, int max) {return SmallIntDistType(gen, min, max);}
#else
  inline SmallIntDistType SmallIntDist(int min, int max) {return SmallIntDistType(static_cast<GeneratorType>(gen),boost::uniform_smallint<>(min,max));}
#endif

/** returns a functor that provides a uniform distribution of integers in the range [\a min, \a max]; 
    if the integers desired are smaller than 10000, SmallIntDist() may be used instead */
#if BOOST_VERSION < 103100
  inline IntDistType IntDist(int min, int max) {return IntDistType(gen, min, max);}
#else
  inline IntDistType IntDist(int min, int max) {return IntDistType(static_cast<GeneratorType>(gen), boost::uniform_int<>(min, max));}
#endif

/** returns a functor that provides a uniform distribution of doubles in the range [\a min, \a max) */
#if BOOST_VERSION < 103100
  inline DoubleDistType DoubleDist(double min, double max) {return DoubleDistType(gen, min, max);}
#else
  inline DoubleDistType DoubleDist(double min, double max) {return DoubleDistType(static_cast<GeneratorType>(gen), boost::uniform_real<>(min, max));}
#endif

/** returns a functor that provides a Gaussian (normal) distribution of doubles centered around \a mean, 
    with standard deviation \a sigma */
#if BOOST_VERSION < 103100
  inline GaussianDistType GaussianDist(double mean, double sigma) {return GaussianDistType(gen, mean, sigma);}
#else
  inline GaussianDistType GaussianDist(double mean, double sigma) {return GaussianDistType(static_cast<GeneratorType>(gen),boost::normal_distribution<>(mean, sigma));}
#endif

/** returns an int from a uniform distribution of small integers in the range [\a min, \a max]; 
    if the integers desired are larger than 10000, use RandInt() instead */
#if BOOST_VERSION < 103100
  inline int RandSmallInt(int min, int max) {return (min == max ? min : SmallIntDistType(gen, min, max)());}
#else
  inline int RandSmallInt(int min, int max) {return (min == max ? min : SmallIntDist(min,max)());}
#endif

/** returns an int from a uniform distribution of integers in the range [\a min, \a max]; 
    if the integers desired are smaller than 10000, RandSmallInt() may be used instead */
#if BOOST_VERSION < 103100
  inline int RandInt(int min, int max) {return (min == max ? min : IntDistType(gen, min, max)());}
#else
  inline int RandInt(int min, int max) {return (min == max ? min : IntDist(min, max)());}
#endif

/** returns a double from a uniform distribution of doubles in the range [0.0, 1.0) */
inline double RandZeroToOne() {return zero_to_one_gen();}

/** returns a double from a uniform distribution of doubles in the range [\a min, \a max) */
#if BOOST_VERSION < 103100
  inline double RandDouble(double min, double max) {return (min == max ? min : DoubleDistType(gen, min, max)());}
#else
  inline double RandDouble(double min, double max) {return (min == max ? min : DoubleDist(min, max)());}
#endif

/** returns a double from a Gaussian (normal) distribution of doubles centered around \a mean, 
    with standard deviation \a sigma */
#if BOOST_VERSION < 103100
  inline double RandGaussian(double mean, double sigma) {return GaussianDistType(gen, mean, sigma)();}
#else
  inline double RandGaussian(double mean, double sigma) {return GaussianDist(mean, sigma)();}
#endif

#endif // _Random_h_
