#include "CheckSums.h"

#if __has_include(<bit>)
#  include <bit>
#endif
#include <float.h>

namespace CheckSums {
    static_assert(!abs(false));
    static_assert(abs(true));
    static_assert(abs(-1) == 1);
    static_assert(abs(0) == 0);
    static_assert(abs(1) == 1);
    static_assert(cmp_less_equal(nlm<uint32_t>(), nlm<uint64_t>()));
    static_assert(cmp_less_equal(nlm<int16_t>(), nlm<uint16_t>()));
    static_assert(fits_in_uint32t<bool>());
    static_assert(fits_in_uint32t<int16_t>());
    static_assert(fits_in_uint32t<uint32_t>());
    static_assert(!fits_in_uint32t<int64_t>());
    static_assert(!fits_in_uint32t<uint64_t>());
    enum class test_enum : int16_t { NEG1 = -1, ZERO, ONE, TWO };
    static_assert(fits_in_uint32t<test_enum>());
    static_assert(abs(test_enum::TWO) == test_enum::TWO);

    static_assert(SeparateParts(-1.5) == std::pair<int64_t, double>{-2, 0.5});
    static_assert(SeparateParts(21.5f) == std::pair<int64_t, float>{21, 0.5f});

    constexpr double ok_frac_ac = 0.0000001;

    static_assert(pow(0) == 1);
    static_assert(pow(0.0) == 1);
    static_assert(abs(pow(1) - natural_log_base) < ok_frac_ac);
    static_assert(abs(pow(5.0) - 148.413159102576) < ok_frac_ac);
    static_assert(abs(pow(-3.14)*pow(3.14) - 1) < ok_frac_ac);
    static_assert(abs(1.0-pow(120.5)/21502307550479587137010156493499279428753405880629141.) < ok_frac_ac);
    static_assert(abs(1.0-pow(200)/722597376812574925817747704218930569735687442852731928403269000000000000000000000000000.) < ok_frac_ac);

    static_assert(pow(12.253, 0) == 1.0);    // special case, should be exact
    static_assert(pow(12.253, 1) == 12.253); // special case, should be exact
    static_assert(pow(2.0, 5) == 32.0);      // power of two, should be exactly represented
    static_assert(pow(2, 5) == 32);
    static_assert(pow(-2, 5) == -32);
    static_assert(pow(4.0, -3) == 1/64.0);   // power of two, should be exactly represented
    static_assert(pow(-4.0, -3) == -1/64.0); // power of two, should be exactly represented
    static_assert(pow(-4, 3) == -64);    // power of two, should be exactly represented
    static_assert(pow(4, 2) == 16);
    static_assert(pow(-4, 2) == 16);

    static_assert(abs(pow10(2.0)/100.0 - 1) < ok_frac_ac);
    static_assert(abs(pow10(0.0) - 1) < ok_frac_ac);
    static_assert(abs(pow10(-1) - 0.1) < ok_frac_ac);
    static_assert(abs(pow10(-4)/0.0001 - 1) < ok_frac_ac);
    static_assert(abs(pow10(0.00000005f)/1.0000001f - 1) < ok_frac_ac);
    static_assert(abs(pow10(-0.00000005f)*1.0000001f - 1) < ok_frac_ac);
    static_assert(abs(pow10(0.9999999f)/9.999997f - 1) < ok_frac_ac);
    static_assert(abs(pow10(-0.9999999f)*9.999997f - 1) < ok_frac_ac);

    static_assert(pow(0.0, 17.0) == 0.0);
    static_assert(pow(1.0f, 17.0f) == 1.0f);
    static_assert(pow(2.0, 2.0) == 4.0);
    static_assert(pow(-2.0, 2.0) == 4.0);
    static_assert(pow(-2.0, 3.0) == -8.0);
    static_assert(abs(pow(8.0, 1/3.0) - 2.0) < ok_frac_ac);
    static_assert(abs(pow(8.0, -1/3.0) - 0.5) < ok_frac_ac);

    static_assert(GetCheckSum(0.0f) == 0);
    static_assert(GetCheckSum(0.0) == 0);
    static_assert(GetCheckSum(1.0f) == 4000000);
    static_assert(GetCheckSum(1.0) == 4000000);
    static_assert(FLT_MIN < 2.0e-38f);
    static_assert(GetCheckSum(2.0e-38f) == 3623010);
    static_assert(GetCheckSum(2.0e-38) == 3623010);
    static_assert(DBL_MIN < 4.0e-308);
    static_assert(GetCheckSum(4.0e-308) == 926020);

    static_assert(GetCheckSum(0.1) == 3990000);
    static_assert(GetCheckSum(0.1f) == 3990000);

    static_assert(GetCheckSum(pow10(-200)) == 2000000);
    static_assert(GetCheckSum(pow10(-1)) == 3990000);
    static_assert(GetCheckSum(pow10(0.0)) == 4000000);
    static_assert(GetCheckSum(pow10(0.0f)) == 4000000);
    static_assert(GetCheckSum(pow10(1.0)) == 4010000);
    static_assert(GetCheckSum(pow10(1.0f)) == 4010000);
    static_assert(GetCheckSum(pow10(0.0001)) == 4000000);
    static_assert(GetCheckSum(pow10(0.01f)) == 4000100);
    static_assert(GetCheckSum(pow10(1.00015)) == 4010001);
    static_assert(GetCheckSum(pow10(1.00015f)) == 4010001);
    static_assert(GetCheckSum(pow10(2)) == 4020000);
    static_assert(GetCheckSum(pow10(200)) == 6000000);

#if !defined(__cpp_lib_constexpr_cmath)
    static_assert(LogNearestInteger(1.0).first == 0);
#endif

    static_assert(abs(log(10.0) - log_base_e_of_10) < ok_frac_ac);

    constexpr double log_base_10_of_e = 1/log_base_e_of_10;

    static_assert(abs(log10(1.0) - 0) < ok_frac_ac);
    static_assert(abs(log10(natural_log_base) - log_base_10_of_e) < ok_frac_ac);
    static_assert(abs(log10(pow10(18.5)) - 18.5) < ok_frac_ac);
    static_assert(abs(log10(pow10(300.5)) - 300.5) < ok_frac_ac);

    static_assert(noexcept(99837 + static_cast<unsigned int>(43.0f)));
    static_assert(DBL_MAX_10_EXP < 400);
    static_assert(FLT_MAX_10_EXP < 40);

#if !defined(__cpp_lib_constexpr_cmath)
    static_assert([](){std::array arr{1, 5, -1, 2, 0, 0}; InPlaceSort(arr); return arr; }() == std::array{-1, 0, 0, 1, 2, 5});
    static_assert([](){std::array arr{1.0, 5.0, -1.0, 2.0, 0.0, 0.0}; InPlaceSort(arr); return arr; }() == std::array{-1.0, 0.0, 0.0, 1.0, 2.0, 5.0});
    static_assert([](){std::array arr{1.0f, 5.0f, -1.0f, 2.0f, 0.0f, 0.0f}; InPlaceSort(arr); return arr; }() == std::array{-1.0f, 0.0f, 0.0f, 1.0f, 2.0f, 5.0f});
#endif

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
        std::array<signed char, ucvals.size()> uc2scvals_retval{};
        for (size_t idx = 0u; idx < ucvals.size(); ++idx)
            uc2scvals_retval[idx] = static_cast<signed char>(ucvals[idx]);
        return uc2scvals_retval;
    }(ucvals);

    constexpr auto sc2ucvals = [](const auto scvals) {
        std::array<unsigned char, scvals.size()> sc2ucvals_retval{};
        for (size_t idx = 0u; idx < scvals.size(); ++idx)
            sc2ucvals_retval[idx] = static_cast<unsigned char>(scvals[idx]);
        return sc2ucvals_retval;
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
    constexpr auto long_chars_csc = GetCheckSum(long_chars_sv);
    static_assert(long_chars_csc == 5816);

    static_assert(CHECKSUM_MODULUS < std::numeric_limits<uint32_t>::max());
    static_assert(CHECKSUM_MODULUS < std::numeric_limits<int32_t>::max());

    static_assert(noexcept(99253 + static_cast<unsigned int>(43.0)));
    static_assert(noexcept(73423 % CHECKSUM_MODULUS));

    template <typename T>
    constexpr bool can_combine = requires(uint32_t& i, const T& t) { CheckSumCombine(i, t); };

    struct DummyPair { int first, second; };
    static_assert(can_combine<DummyPair>);

    struct DummyEmpty {};
    static_assert(!can_combine<DummyEmpty>);
}
