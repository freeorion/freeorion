// -*- C++ -*-
#ifndef _Random_h_
#define _Random_h_

// HACK! this keeps gcc 3.2 from barfing when it sees "typedef long long uint64_t;"
// in boost/cstdint.h when compiling under windows
#ifdef WIN32
#  define BOOST_MSVC -1
#endif
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_smallint.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/uniform_01.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/random/normal_distribution.hpp>
#ifdef WIN32
#  undef BOOST_MSVC
#endif

#include <ctime>

/** \file Random.h
    A collection of robust and portable random number generation functors and functions.
    If you need to generate one or two random numbers, prefer the functions (e.g. RandomInt()).
    If you need to generate a large volume of random numbers with the same parameterization,
    generate a functor (e.g. with a call to IntDist()) and then call the functor repeatedly to
    generate the numbers.  This eliminates the overhead associated with repeatedly contructing 
    distributions, when you call the Random*() functions. */

typedef boost::mt19937                            GeneratorType;
typedef boost::uniform_smallint<GeneratorType>    SmallIntDistType;
typedef boost::uniform_int<GeneratorType>         IntDistType;
typedef boost::uniform_real<GeneratorType>        DoubleDistType;
typedef boost::normal_distribution<GeneratorType> GaussianDistType;

namespace {
GeneratorType gen; // the one random number generator driving the distributions below
boost::uniform_01<GeneratorType> zero_to_one_gen(gen);
}

/** seeds the underlying random number generator used to drive all random number distributions */
//inline void Seed(unsigned int seed) {gen.seed(seed);}

/** seeds the underlying random number generator used to drive all random number distributions with 
    the current clock time */
inline void ClockSeed() {gen.seed(static_cast<unsigned int>(std::time(0)));}

/** returns a functor that provides a uniform distribution of small integers in the range [\a min, \a max]; 
    if the integers desired are larger than 10000, use IntDist() instead */
inline SmallIntDistType SmallIntDist(int min, int max) {return SmallIntDistType(gen, min, max);}

/** returns a functor that provides a uniform distribution of integers in the range [\a min, \a max]; 
    if the integers desired are smaller than 10000, SmallIntDist() may be used instead */
inline IntDistType IntDist(int min, int max) {return IntDistType(gen, min, max);}

/** returns a functor that provides a uniform distribution of doubles in the range [\a min, \a max) */
inline DoubleDistType DoubleDist(double min, double max) {return DoubleDistType(gen, min, max);}

/** returns a functor that provides a Gaussian (normal) distribution of doubles centered around \a mean, 
    with standard deviation \a sigma */
inline GaussianDistType GaussianDist(double mean, double sigma) {return GaussianDistType(gen, mean, sigma);}

/** returns an int from a uniform distribution of small integers in the range [\a min, \a max]; 
    if the integers desired are larger than 10000, use RandInt() instead */
inline int RandSmallInt(int min, int max) {return (min == max ? min : SmallIntDistType(gen, min, max)());}

/** returns an int from a uniform distribution of integers in the range [\a min, \a max]; 
    if the integers desired are smaller than 10000, RandSmallInt() may be used instead */
inline int RandInt(int min, int max) {return (min == max ? min : IntDistType(gen, min, max)());}

/** returns a double from a uniform distribution of doubles in the range [0.0, 1.0) */
inline double RandZeroToOne() {return zero_to_one_gen();}

/** returns a double from a uniform distribution of doubles in the range [\a min, \a max) */
inline double RandDouble(double min, double max) {return (min == max ? min : DoubleDistType(gen, min, max)());}

/** returns a double from a Gaussian (normal) distribution of doubles centered around \a mean, 
    with standard deviation \a sigma */
inline double RandGaussian(double mean, double sigma) {return GaussianDistType(gen, mean, sigma)();}

#endif // _Random_h_
