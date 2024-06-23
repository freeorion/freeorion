#ifndef _Random_h_
#define _Random_h_

#if defined(_MSC_VER) && defined(int64_t)
#undef int64_t
#endif

#include <array>
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

namespace RandomImpl {
    inline constexpr uint16_t counter_val =
#if defined(__COUNTER__)
        __COUNTER__ % 8;
#else
        1;
#endif

    // convert time as string like "23:59:59" to an int
    inline constexpr uint32_t TimeToInt(std::array<char, 9> t) noexcept
    { return ((t[7]-'0') + 10*(t[6]-'0')) + 60*((t[4]-'0') + 10*(t[3]-'0')) + 3600*((t[1]-'0') + 10*(t[0]-'0')); }

    // shuffle bits in number
    inline constexpr uint32_t hash(auto val_in) {
        auto val = static_cast<uint32_t>(val_in);
        val ^= 0x5A5C3AF5;

        // copied from Boost "hash_mix for 32 bit size_t"
        constexpr uint32_t m1 = 0x21f0aaad;
        constexpr uint32_t m2 = 0x735a2d97;
        val ^= val >> 16;
        val *= m1;
        val ^= val >> 15;
        val *= m2;
        val ^= val >> 15;

        return val;
    }
    // generate a compile-time PRNG int that is seeded differently for each second of the day.
    // \a n indicates which value for the current seed to return.
    inline constexpr uint32_t CxPRNG(int16_t n = counter_val, std::array<char, 9> seed = std::array<char, 9>{__TIME__}) {
        uint32_t retval{TimeToInt(seed)};
        for (; n > 0; --n)
            retval = hash(retval);
        return retval;
    }
}

/** returns an int from a uniform distribution of integers in the range [\a min, \a max]; */
FO_COMMON_API int RandInt(int min, int max);

/** compile-time "random" time-seeded int from uniform distribution in the range [\a min, \a max] */
inline constexpr int32_t RandIntCx(const int32_t min, const int32_t max,
                                   const int16_t n = RandomImpl::counter_val) noexcept
{
    if (min >= max)
        return min;
    const auto range = static_cast<uint32_t>(max - min);
    const auto val_in_range = RandomImpl::CxPRNG(n) % (range + 1);
    return min + static_cast<int32_t>(val_in_range);
}
inline constexpr int32_t RandIntCx(const int32_t min, const int32_t max, const int16_t n,
                                   const std::array<char, 9> seed = std::array<char, 9>{__TIME__}) noexcept
{
    if (min >= max)
        return min;
    const auto range = static_cast<uint32_t>(max - min);
    const auto val_in_range = RandomImpl::CxPRNG(n, seed) % (range + 1);
    return min + static_cast<int32_t>(val_in_range);
}
inline constexpr bool RandBoolCx(int16_t n = RandomImpl::counter_val) noexcept
{ return RandomImpl::CxPRNG(n) % 2 == 0; }

/** returns a double from a uniform distribution of doubles in the range [0.0, 1.0) */
FO_COMMON_API double RandZeroToOne();

/** compile-time random double from nearly-uniform distribution in the range [0.0, 1.0);
  * \a n indicates which value in the distribution to return. */
inline constexpr double RandZeroToOneCx(int16_t n = 4) noexcept {
    const uint32_t arbitrary_prime = 31397;
    const auto val = RandomImpl::CxPRNG(n) % arbitrary_prime;
    return static_cast<double>(val) / arbitrary_prime;
}

/** returns a double from a uniform distribution of doubles in the range [\a min, \a max) */
FO_COMMON_API double RandDouble(double min, double max);

/** compile-time random double from nearly-uniform distribution in the range [min, max);
  * \a n indicates which value in the distribution to return. */
inline constexpr double RandDoubleCx(double min, double max, int16_t n = 5) noexcept {
    if (min >= max)
        return min;
    const auto range = max - min;
    return min + RandZeroToOneCx(n)*range;
}

/** returns a double from a Gaussian (normal) distribution of doubles centered around \a mean,
    with standard deviation \a sigma */
FO_COMMON_API double RandGaussian(double mean, double sigma);

/** shuffles contents of container */
FO_COMMON_API void RandomShuffle(std::vector<uint8_t>& c);
FO_COMMON_API void RandomShuffle(std::vector<int>& c);


#endif
