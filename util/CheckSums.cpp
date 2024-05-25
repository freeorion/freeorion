#include "CheckSums.h"

#include <algorithm>
#include <array>
#if __has_include(<bit>)
#  include <bit>
#endif
#include <cassert>
#include <cmath>
#include <float.h>
#include <limits>
#include <numeric>
#include <stdexcept>

namespace {
    constexpr double log_base_e_of_10 = 2.30258509299404568401799145;
    constexpr double log_base_10_of_e = 1/log_base_e_of_10;
    constexpr double natural_log_base = 2.71828182845904523536028747;

    constexpr auto CXAbs(auto in) noexcept { return in >= 0 ? in : -in; }

    constexpr double CXPow(double base, unsigned int exp) {
        if (exp == 0)
            return 1.0;
        if (exp == 1)
            return base;
        return CXPow(base, exp / 2) * CXPow(base, exp - exp/2);
    }
    static_assert(CXPow(12.253, 0) == 1.0);
    static_assert(CXPow(12.253, 1) == 12.253);
    static_assert(CXPow(2.0, 5) == 32.0);
    static_assert(CXPow(4.0, 3) == 64.0);

    constexpr uint64_t FacInt(uint8_t x) noexcept
    { return (x <= 1) ? 1 :FacInt(x-1)*x; }
    constexpr double Fac(uint8_t x) noexcept {
        if (x <= 20)
            return FacInt(x);
        double accum = static_cast<double>(FacInt(20));
        for (auto n = 21; n <= x; ++n)
            accum *= n;
        return accum;
    }
    static_assert(Fac(5) == 120.0);

    constexpr void InPlaceSort(auto& arr) {
#if defined(__cpp_lib_constexpr_algorithms)
        std::sort(arr.begin(), arr.end());
#else
        if (!std::is_constant_evaluated()) {
            std::sort(arr.begin(), arr.end());
        } else {
            for (auto it = arr.begin(); it != arr.end(); ++it)
                std::swap(*it, *std::min_element(it, arr.end()));
        }
#endif
    }

    constexpr double CXPow(const double exp) {
        std::array<double, 64> scratch{};
        for (uint8_t n = 0; n < scratch.size(); ++n)
            scratch[n] = CXPow(exp,n)/Fac(n);
        InPlaceSort(scratch);
#if defined(__cpp_lib_constexpr_numeric)
        return std::accumulate(scratch.begin(), scratch.end(), 0.0);
#else
        double accum = 0.0;
        for (const auto& s : scratch)
            accum += s;
        return accum;
#endif
    }
    static_assert(CXAbs(CXPow(0) - 1) < 0.0000001);
    static_assert(CXAbs(CXPow(1) - natural_log_base) < 0.0000001);
    static_assert(CXAbs(CXPow(5.0) - 148.413159102576) < 0.0000001);
    static_assert(CXAbs(CXPow(-3.14)*CXPow(3.14) - 1) < 0.0000001);

    constexpr double CXPow10(const double x) noexcept
    { return CXPow(x*log_base_e_of_10); }
    static_assert(CXAbs(CXPow10(2.0)/100.0 - 1) < 0.0000001);

    constexpr std::pair<double, std::size_t> CXLog10Impl(const double x, uint8_t max_iter = 20) {
        if (x < 0)
            throw std::invalid_argument("can't take log10 of negative number");
        if (x == 0)
            throw std::invalid_argument("can't take log10 of 0");
        if (x == 1)
            return {0.0, 0};

        constexpr auto initial_guess = [](auto x_in) noexcept {
            std::decay_t<decltype(x_in)> guess = 0;
            while (x_in > 3.17) { x_in /= 10; ++guess; }  // 3.16 ~= 10^0.5
            while (x_in < 0.315) { x_in *= 10; --guess; } // 0.316 ~= 10^-0.5
            return guess;
        };
        static_assert(initial_guess(1) == 0);
        static_assert(initial_guess(100) == 2);
        static_assert(initial_guess(2.0) < 1.30 && initial_guess(2.0) > -0.7);
        static_assert(initial_guess(3.0) < 1.47 && initial_guess(3.0) > -0.53);
        static_assert(initial_guess(5.0) < 1.70 && initial_guess(5.0) > -0.3);
        static_assert(initial_guess(8.0) < 1.90 && initial_guess(8.0) > -0.1);
        static_assert(initial_guess(12.0) < 2.08 && initial_guess(12.0) > 0.08);

        // determine initial search bounds
        double guess = initial_guess(x);

        // want to find y = log10(x), ie. find y such that x = 10^y
        // equivalent to finding root f(y) = x - 10^y = 0
        // f(y) = x - exp(ln(10)*y)
        // f'(y) = -ln(10)*exp(ln(10)*y) = -ln(10)*10^y

        // Newton: guess y_n+1 - y_n = -f(y_n)/f'(y_n)
        //                           = -(x - 10^y_n) / [-ln(10)*10^y_n]
        //                           = (1/ln(10))*(x - 10^y_n)/(10^y_n)
        //                           = (1/ln(10))*(x/(10^y_n) - 1)

        constexpr double one_over_ln10 = 0.434294481903251827651128918;

        std::size_t iterations = 0;
        for (; iterations < max_iter; ++iterations) {
            auto adj_factor = one_over_ln10*(x/CXPow10(guess) - 1);
            if (adj_factor == 0)
                break;
            guess += adj_factor;
            ++iterations;
        }
        return {guess, iterations};
    }
    constexpr double CXLog10(const double x) { return CXLog10Impl(x).first; }

    static_assert(CXAbs(CXLog10(1) - 0) < 0.0000001);
    static_assert(CXAbs(CXLog10(natural_log_base) - log_base_10_of_e) < 0.0000001);
    constexpr auto bignum = CXPow10(20);
    static_assert(CXAbs(CXLog10(CXPow10(18.5)) - 18.5) < 0.00001);
}

namespace CheckSums {
    constexpr auto csc = [](const auto in) noexcept {
        constexpr auto csc_ = [](const auto in, const auto csc_) noexcept {
            if constexpr (std::is_arithmetic_v<decltype(in)>) {
                uint32_t sum = 0;
                CheckSumCombine(sum, in);
                return sum;
            } else {
                std::array<uint32_t, in.size()> sums = {0};
                for (size_t idx = 0; idx < sums.size(); ++idx)
                    sums[idx] = csc_(in[idx], csc_);
                return sums;
            }
        };

        return csc_(in, csc_);
    };
    static_assert(csc('q') == 'q');

    static_assert(CHAR_BIT == 8);
    constexpr auto ucvals = std::array<unsigned char, 15>{0,1,40,'q',120,126,127,128,129,130,200,250,253,254,255};
    constexpr auto scvals = std::array<signed char, 15>  {0,1,40,'q',120,126,127,-128,-127,-126,-56,-6,-3,-2,-1};
#if defined(__cpp_lib_bit_cast)
    static_assert(std::bit_cast<decltype(ucvals)>(scvals) == ucvals);
    static_assert(std::bit_cast<decltype(scvals)>(ucvals) == scvals);
    static_assert(std::bit_cast<std::array<char, ucvals.size()>>(ucvals) ==
                  std::bit_cast<std::array<char, scvals.size()>>(scvals));
#endif
    static_assert(std::is_convertible_v<unsigned char, char> || std::is_convertible_v<signed char, char>);
    static_assert(std::is_convertible_v<char, unsigned char> || std::is_convertible_v<char, signed char>);

    constexpr auto uc2scvals = [](const auto ucvals){
        std::array<signed char, ucvals.size()> uc2scvals{};
        for (size_t idx = 0u; idx < ucvals.size(); ++idx)
            uc2scvals[idx] = static_cast<signed char>(ucvals[idx]);
        return uc2scvals;
    }(ucvals);

    constexpr auto sc2ucvals = [](const auto scvals) {
        std::array<unsigned char, scvals.size()> sc2ucvals{};
        for (size_t idx = 0u; idx < scvals.size(); ++idx)
            sc2ucvals[idx] = static_cast<unsigned char>(scvals[idx]);
        return sc2ucvals;
    }(scvals);

    constexpr auto csc1 = csc(ucvals);
    constexpr auto csc2 = csc(sc2ucvals);
    constexpr auto csc3 = csc(uc2scvals);
    constexpr auto csc4 = csc(scvals);
    static_assert(csc1 == csc2);
    static_assert(csc2 == csc3);
    static_assert(csc3 == csc4);
    static_assert(csc1[3] == 'q');
    static_assert([]() {
                    for (size_t idx = 0u; idx < csc1.size(); ++idx)
                        if (csc1[idx] != ucvals[idx])
                            return false;
                    return true;
                  }());

#if defined(__cpp_lib_char8_t)
    constexpr std::u8string_view long_chars = u8"αbåオーガитیای مجهو ";
    constexpr auto long_chars_arr = []() {
        std::array<std::string_view::value_type, long_chars.size()> retval{};
        for (std::size_t idx = 0; idx < retval.size(); ++idx)
            retval[idx] = long_chars[idx];
        return retval;
    }();
    constexpr std::string_view long_chars_sv(long_chars_arr.data(), long_chars_arr.size());
#else
    constexpr std::string_view long_chars_sv = u8"αbåオーガитیای مجهو";
#endif
    constexpr auto char3 = long_chars_sv[3];
    constexpr auto char3csc = csc(char3);
    static_assert(char3csc == 195u);
    constexpr auto char13 = long_chars_sv[13];
    constexpr auto char13csc = csc(char13);
    static_assert(char13csc == 172u);
    constexpr auto char24 = long_chars_sv[24];
    constexpr auto char24csc = csc(char24);
    static_assert(char24csc == 32u);
    static_assert(long_chars_sv.size() == 34);
    constexpr auto long_chars_csc = []() {
        uint32_t sum = 0;
        CheckSumCombine(sum, long_chars_sv);
        return sum;
    }();
    static_assert(long_chars_csc == 5816);


    static_assert(CHECKSUM_MODULUS < std::numeric_limits<uint32_t>::max());
    static_assert(CHECKSUM_MODULUS < std::numeric_limits<int32_t>::max());

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
        if (std::is_constant_evaluated())
            sum += static_cast<unsigned int>((CXLog10(CXAbs(t)) + 400.0) * 10000.0);
        else
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
        if (std::is_constant_evaluated())
            sum += static_cast<unsigned int>((CXLog10(CXAbs(t)) + 40.0f) * 100000.0f);
        else
            sum += static_cast<unsigned int>((std::log10(std::abs(t)) + 40.0f) * 100000.0f);
        sum %= CHECKSUM_MODULUS;
    }
}
