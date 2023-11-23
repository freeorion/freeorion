#include "CheckSums.h"

#include <array>
#if __has_include(<bit>)
#  include <bit>
#endif
#include <cassert>
#include <cmath>
#include <float.h>
#include <limits>

namespace CheckSums {
    auto csc_ = [](const auto in, const auto csc_) noexcept {
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
    auto csc = [](const auto in) noexcept { return csc_(in, csc_); };
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
