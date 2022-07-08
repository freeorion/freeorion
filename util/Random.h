#ifndef _Random_h_
#define _Random_h_

#if defined(_MSC_VER) && defined(int64_t)
#undef int64_t
#endif

#include <random>
#include <ctime>

#include "Export.h"

/** \file
 *
 * A collection of robust and portable random number generation functions that
 * share an underlying random generator and which are guarded with locks for
 * thread safety.
 */

/** seeds the underlying random number generator used to drive all random number distributions */
FO_COMMON_API void Seed(unsigned int seed);

/** seeds the underlying random number generator used to drive all random number distributions with
    the current clock time */
FO_COMMON_API void ClockSeed();

/** returns an int from a uniform distribution of integers in the range [\a min, \a max]; */
FO_COMMON_API int RandInt(int min, int max);

/** returns a double from a uniform distribution of doubles in the range [0.0, 1.0) */
FO_COMMON_API double RandZeroToOne();

/** returns a double from a uniform distribution of doubles in the range [\a min, \a max) */
FO_COMMON_API double RandDouble(double min, double max);

/** returns a double from a Gaussian (normal) distribution of doubles centered around \a mean,
    with standard deviation \a sigma */
FO_COMMON_API double RandGaussian(double mean, double sigma);

/** shuffles contents of container */
FO_COMMON_API void RandomShuffle(std::vector<uint8_t>& c);
FO_COMMON_API void RandomShuffle(std::vector<int>& c);


#endif
