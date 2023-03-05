#include "CheckSums.h"

#include <cassert>
#include <cmath>
#include <float.h>
#include <limits>

namespace CheckSums {
    void CheckSumCombine(uint32_t& sum, const char* s)
    { CheckSumCombine(sum, std::string(s)); }

    static_assert(CHECKSUM_MODULUS < std::numeric_limits<uint32_t>::max());
    static_assert(CHECKSUM_MODULUS < std::numeric_limits<int32_t>::max());

    void CheckSumCombine(uint32_t& sum, const std::string& c) {
        for (auto& t : c)
            CheckSumCombine(sum, t);
        sum += c.size();
        sum %= CHECKSUM_MODULUS;
    }

    static_assert(noexcept(99253 + static_cast<unsigned int>(43.0)));
    static_assert(noexcept(73423 % CHECKSUM_MODULUS));
    void CheckSumCombine(uint32_t& sum, double t) noexcept(csc_double) {
        static_assert(DBL_MAX_10_EXP < 400);
        if (t == 0.0)
            return;
        // biggest and smallest possible double should be ~10^(+/-308)
        // taking log gives a number in the range +/- 309
        // adding 400 gives numbers in the range ~0 to 800
        // multiplying by 10'000 gives numbers in the range ~0 to 8'000'000
        sum += static_cast<unsigned int>((std::log10(std::abs(t)) + 400.0) * 10000.0);
        sum %= CHECKSUM_MODULUS;
    }

    static_assert(noexcept(99837 + static_cast<unsigned int>(43.0f)));
    void CheckSumCombine(uint32_t& sum, float t) noexcept {
        static_assert(FLT_MAX_10_EXP < 40);
        if (t == 0.0f)
            return;
        // biggest and smallest possible float should be ~10^(+/-38)
        // taking log gives a number in the range +/- 39
        // adding 40 gives numbers in the range ~0 to 80
        // multiplying by 100'000 gives numbers in the range ~0 to 8'000'000
        sum += static_cast<unsigned int>((std::log10(std::abs(t)) + 40.0f) * 100000.0f);
        sum %= CHECKSUM_MODULUS;
    }
}
