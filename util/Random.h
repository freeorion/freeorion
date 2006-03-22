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
#include <boost/random/variate_generator.hpp>
#include <ctime>

/** \file Random.h
    A collection of robust and portable random number generation functors and functions.
    If you need to generate one or two random numbers, prefer the functions (e.g. RandomInt()).
    If you need to generate a large volume of random numbers with the
    same parameterization,
    generate a functor (e.g. with a call to IntDist()) and then call the functor repeatedly to
    generate the numbers.  This eliminates the overhead associated with repeatedly contructing 
    distributions, when you call the Random*() functions. */

typedef boost::mt19937                                                          GeneratorType;
typedef boost::variate_generator<GeneratorType&, boost::uniform_smallint<> >    SmallIntDistType;
typedef boost::variate_generator<GeneratorType&, boost::uniform_int<> >         IntDistType;
typedef boost::variate_generator<GeneratorType&, boost::uniform_real<> >        DoubleDistType;
typedef boost::variate_generator<GeneratorType&, boost::normal_distribution<> > GaussianDistType;

/** seeds the underlying random number generator used to drive all random number distributions */
void Seed(unsigned int seed);

/** seeds the underlying random number generator used to drive all random number distributions with 
    the current clock time */
void ClockSeed();

/** returns a functor that provides a uniform distribution of small
    integers in the range [\a min, \a max]; if the integers desired
    are larger than 10000, use IntDist() instead */
SmallIntDistType SmallIntDist(int min, int max);

/** returns a functor that provides a uniform distribution of integers in the range [\a min, \a max]; 
    if the integers desired are smaller than 10000, SmallIntDist() may be used instead */
IntDistType IntDist(int min, int max);

/** returns a functor that provides a uniform distribution of doubles in the range [\a min, \a max) */
DoubleDistType DoubleDist(double min, double max);

/** returns a functor that provides a Gaussian (normal) distribution of doubles centered around \a mean, 
    with standard deviation \a sigma */
GaussianDistType GaussianDist(double mean, double sigma);

/** returns an int from a uniform distribution of small integers in the range [\a min, \a max]; 
    if the integers desired are larger than 10000, use RandInt() instead */
int RandSmallInt(int min, int max);

/** returns an int from a uniform distribution of integers in the range [\a min, \a max]; 
    if the integers desired are smaller than 10000, RandSmallInt() may be used instead */
int RandInt(int min, int max);

/** returns a double from a uniform distribution of doubles in the range [0.0, 1.0) */
double RandZeroToOne();

/** returns a double from a uniform distribution of doubles in the range [\a min, \a max) */
double RandDouble(double min, double max);

/** returns a double from a Gaussian (normal) distribution of doubles centered around \a mean, 
    with standard deviation \a sigma */
double RandGaussian(double mean, double sigma);

#endif // _Random_h_
