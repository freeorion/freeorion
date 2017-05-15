#include "CheckSums.h"

#include <cassert>
#include <cmath>
#include <float.h>

namespace CheckSums {
    void CheckSumCombine(unsigned int& sum, const char* s)
    { CheckSumCombine(sum, std::string(s)); }

    void CheckSumCombine(unsigned int& sum, const std::string& c) {
        TraceLogger() << "CheckSumCombine(std::string): " << c;
        for (auto& t : c)
            CheckSumCombine(sum, t);
        sum += c.size();
        sum %= CHECKSUM_MODULUS;
    }

    void CheckSumCombine(unsigned int& sum, double t) {
        assert(DBL_MAX_10_EXP < 400);
        if (t == 0.0)
            return;
        // biggest and smallest possible double should be ~10^(+/-308)
        // taking log gives a number in the range +/- 309
        // adding 400 gives numbers in the range ~0 to 800
        // multiplying by 10'000 gives numbers in the range ~0 to 8'000'000
        sum += static_cast<unsigned int>((std::log10(std::abs(t)) + 400.0) * 10000.0);
        sum %= CHECKSUM_MODULUS;
    }

    void CheckSumCombine(unsigned int& sum, float t) {
        assert(FLT_MAX_10_EXP < 40);
        if (t == 0.0f)
            return;
        // biggest and smallest possible float should be ~10^(+/-38)
        // taking log gives a number in the range +/- 39
        // adding 400 gives numbers in the range ~0 to 80
        // multiplying by 100'000 gives numbers in the range ~0 to 8'000'000
        sum += static_cast<unsigned int>((std::log10(std::abs(t)) + 40.0f) * 100000.0f);
        sum %= CHECKSUM_MODULUS;
    }
}
