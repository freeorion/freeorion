#ifndef _CheckSums_h_
#define _CheckSums_h_

#include "Export.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <limits.h>
#include <memory>
#include <numeric>
#include <utility>
#include <stdexcept>
#include <string_view>
#include <type_traits>

namespace CheckSums {
#if !defined(__cpp_lib_integer_comparison_functions)
    constexpr bool cmp_less_equal(const auto& l, const auto& r) noexcept { return l <= r; }
#else
    using std::cmp_less_equal;
#endif
    constexpr bool cmp_less_equal(const auto& l, bool r) noexcept { return l <= decltype(l)(r ? 1u : 0u); }
    constexpr bool cmp_less_equal(bool l, const auto& r) noexcept { return decltype(r)(l ? 1u : 0u) <= r; }

    template <typename T>
    constexpr auto abs(const T& i) noexcept(std::is_arithmetic_v<std::decay_t<T>>)
    {
        using I_t = std::decay_t<T>;
        if constexpr (std::is_unsigned_v<I_t>) {
            return i;

        } else if constexpr (std::is_enum_v<I_t>) {
            using ULT_t = std::underlying_type_t<I_t>;
            if constexpr (std::is_signed_v<ULT_t>)
                if (static_cast<ULT_t>(i) < 0)
                    throw std::invalid_argument("abs given enum with negative underlying value");
            return i;

        } else if constexpr (std::is_arithmetic_v<I_t>) {
#if defined(__cpp_lib_constexpr_cmath)
            return std::abs(i);
#else
            return (i >= 0) ? i : -i;
#endif
        } else {
            throw std::invalid_argument("abs passed unrecognized type");
        }
    }

    // ensure type fits into CheckSum output
    template <typename T> requires(std::is_integral_v<std::decay_t<T>>)
    consteval auto nlm() noexcept { return std::numeric_limits<std::decay_t<T>>::max(); }

    template <typename T> requires(std::is_enum_v<std::decay_t<T>>)
    consteval auto nlm() noexcept { return nlm<std::underlying_type_t<T>>(); }

    template <typename T> requires(std::is_integral_v<std::decay_t<T>> || std::is_enum_v<std::decay_t<T>>)
    consteval bool fits_in_uint32t() noexcept { return cmp_less_equal(nlm<T>(), nlm<uint32_t>()); }

    inline constexpr uint32_t CHECKSUM_MODULUS = 10000000U;    // reasonably big number that should be well below UINT_MAX, which is ~4.29x10^9 for 32 bit unsigned int
    static_assert(CHECKSUM_MODULUS < UINT_MAX/4);

#if !defined(__cpp_lib_constexpr_cmath)
    constexpr double log_base_e_of_10 = 2.30258509299404568401799145;
    constexpr double natural_log_base = 2.71828182845904523536028747;

    template <typename B, typename E> requires std::is_integral_v<E>
    constexpr B pow(B base, E exp)
    {
        if constexpr (std::is_signed_v<E>) {
            if constexpr (std::is_floating_point_v<B>) {
                if (exp < 0)
                    return 1/pow(base, static_cast<std::make_unsigned_t<E>>(-exp));
            } else {
                if (exp < 0)
                    throw std::invalid_argument("can't do integral exponent of negative power");
            }
            return pow(base, static_cast<std::make_unsigned_t<E>>(exp));

        } else {
            if (base == 0 && exp == 0)
                throw std::invalid_argument("0^0 is undefined");
            if (exp == 0) return 1;
            if (exp == 1) return base;
            B accum = 1;
            while (exp-->0)
                accum *= base;
            return accum;
        }
    }

    template <typename T = double, typename E>
        requires std::is_floating_point_v<T> && std::is_integral_v<E>
    constexpr T pow(E exp) noexcept
    { return pow(static_cast<T>(natural_log_base), exp); }

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

    template <typename E = double> requires std::is_floating_point_v<std::decay_t<E>>
    constexpr E PowTaylorSeries(const E exp)
    {
        if (exp == 0)
            return 1;
        if (exp > 1 || exp < 0)
            throw std::invalid_argument("exponent to power Taylor series should be >= 0 and <= 1");
        std::array<E, 16> scratch{};
        scratch[0] = 1;
        E accum = 1;

        for (std::size_t n = 1u; n < scratch.size(); ++n) {
            if (accum*exp < n*std::numeric_limits<E>::min())
                break; // when values get too small, constexpr calculations aren't happy...
            accum *= (exp/n);
            scratch[n] = accum;
        }
        InPlaceSort(scratch);
#if defined(__cpp_lib_constexpr_numeric)
        return std::accumulate(scratch.begin(), scratch.end(), E{0});
#else
        accum = 0;
        for (const auto& s : scratch)
            accum += s;
        return accum;
#endif
    }

    constexpr auto SeparateParts(auto exp) noexcept {
        // skipping safety checks about whether exp fits in int64_t
        int64_t int_exp = static_cast<int64_t>(exp >= 0 ? exp : (exp - 1));
        return std::pair{int_exp, exp - int_exp};
    }

    template <typename E = double> requires std::is_floating_point_v<std::decay_t<E>>
    constexpr E pow(E exp)
    {
        const auto [int_exp, frac_exp] = SeparateParts(exp);
        return pow(static_cast<E>(natural_log_base), int_exp) * PowTaylorSeries(frac_exp);
    }

    template <typename T = double, typename E> requires std::is_integral_v<E>
    constexpr T pow10(E x) noexcept
    { return pow(static_cast<T>(10), x); }

    template <typename E = double> requires std::is_floating_point_v<std::decay_t<E>>
    constexpr E pow10(E x)
    { return pow(x*static_cast<E>(log_base_e_of_10)); }

    template <typename E = double> requires std::is_floating_point_v<std::decay_t<E>>
    constexpr auto LogNearestInteger(E x) noexcept {
        const E base{static_cast<std::decay_t<decltype(x)>>(natural_log_base)};
        int32_t int_log = 0;
        while (x > 1) {
            x /= base;
            ++int_log;
        }
        while (x < 1) {
            x *= base;
            --int_log;
        }
        return std::pair{int_log, x};
    }

    template <typename E = double> requires std::is_floating_point_v<std::decay_t<E>>
    constexpr E log(E x, std::size_t max_iter = 20)
    {
        if (x <= 0) throw std::invalid_argument("can't take log of non-positive number");
        auto [int_log, frac_part] = LogNearestInteger(x);
        if (frac_part == 1) return static_cast<E>(int_log);

        // want to find y = log(x), ie. find y such that x = e^y
        // equivalent to finding root f(y) = x - e^y = 0
        // f(y) = x - exp(y)
        // f'(y) = -exp(y)
        //
        // Newton: guess y_n+1 - y_n = -f(y_n)/f'(y_n)
        //                           = -(x - e^y_n) / (-e^y_n)
        //                           =  (x / e^y_n) - 1
        E guess = static_cast<E>(int_log);
        for (std::size_t iterations = 0; iterations < max_iter; ++iterations) {
            auto adj_factor = (x/pow(guess) - 1);
            if (adj_factor == 0)
                break;
            guess += adj_factor;
        }
        return guess;
    }

    template <typename E> requires std::is_integral_v<std::decay_t<E>>
    constexpr E log(E x) { return static_cast<E>(log(static_cast<double>(x))); }

    template <typename E> requires std::is_floating_point_v<std::decay_t<E>>
    constexpr E log10(const E x, std::size_t max_iter = 20)
    { return log(x, max_iter) / static_cast<E>(log_base_e_of_10); }

    template <typename B = double, typename E = double> requires std::is_floating_point_v<E>
    constexpr E pow(B base, E exp)
    {
        if (base == 0 && exp <= 0)
            throw std::invalid_argument("0^(<=0) is undefined");
        if (base == 0 || base == 1)
            return base;
        if (static_cast<int64_t>(exp) == exp)
            return pow(base, static_cast<int64_t>(exp));
        if (base < 0)
            throw std::invalid_argument("(-)^(fraction) might require complex values, not handled");

        // base is positive, exp is non-integer
        return pow(exp*log(static_cast<E>(base)));
    }

#else
    using std::abs;
    using std::log;
    using std::log10;
    using std::pow;
    constexpr double pow10(double exp) { return std::pow(10.0, exp); }
    constexpr float pow10(float exp) { return std::pow(10.0f, exp); }
#endif

    // integeral types
    template <typename T> requires (std::is_integral_v<T>)
    constexpr void CheckSumCombine(uint32_t& sum, T t) noexcept
    {
        static_assert(noexcept(sum + static_cast<uint32_t>(t)));
        static_assert(noexcept(sum % CHECKSUM_MODULUS));
        if constexpr (!fits_in_uint32t<T>())
            t %= static_cast<T>(CHECKSUM_MODULUS);
        if constexpr (std::is_signed_v<std::decay_t<T>>)
            t = abs(t);
        sum += static_cast<uint32_t>(t);
        sum %= CHECKSUM_MODULUS;
    }
    template <>
    constexpr void CheckSumCombine(uint32_t& sum, signed char t) noexcept
    {
        static_assert(noexcept(sum + static_cast<uint32_t>(static_cast<unsigned char>(t))));
        static_assert(noexcept(sum % CHECKSUM_MODULUS));
        CheckSumCombine(sum, static_cast<uint32_t>(static_cast<unsigned char>(t)));
    }
    template <>
    constexpr void CheckSumCombine(uint32_t& sum, char t) noexcept
    {
        if constexpr (std::is_signed_v<char>)
            CheckSumCombine<signed char>(sum, t);
        else
            CheckSumCombine<unsigned char>(sum, t);
    }

    // floating point types types
    template <typename T> requires std::is_floating_point_v<std::decay_t<T>>
    constexpr void CheckSumCombine(uint32_t& sum, T t)
        noexcept(noexcept(std::log10(std::abs(T{1}))))
    {
        if (t == 0.0)
            return;
        // biggest and smallest possible double should be ~10^(+/-308)
        // taking log gives a number in the range +/- 309
        // adding 400 gives numbers in the range ~0 to 800
        // multiplying by 10'000 gives numbers in the range ~0 to 8'000'000
        auto abs_log10 = std::is_constant_evaluated() ? log10(abs(t)) : std::log10(std::abs(t));
        CheckSumCombine(sum, static_cast<uint32_t>((abs_log10 + 400)*10'000));
    }

    // strings
    constexpr void CheckSumCombine(uint32_t& sum, std::string_view sv)
        noexcept(noexcept(CheckSumCombine(sum, sv.front())) && noexcept(CheckSumCombine(sum, sv.size())))
    {
        for (auto t : sv)
            CheckSumCombine(sum, t);
        CheckSumCombine(sum, sv.size());
    }
    constexpr void CheckSumCombine(uint32_t& sum, const char* s)
        noexcept(noexcept(CheckSumCombine(sum, std::string_view{s})))
    { CheckSumCombine(sum, std::string_view{s}); }

    // enums
    template <typename EnumT> requires (std::is_enum_v<EnumT>)
    constexpr void CheckSumCombine(uint32_t& sum, EnumT t) noexcept
    { CheckSumCombine(sum, static_cast<int>(t) + 10); }

    constexpr void CheckSumCombine(uint32_t& sum, const auto& v)
        requires(!std::is_integral_v<std::decay_t<decltype(v)>> &&
                 !std::is_floating_point_v<std::decay_t<decltype(v)>> &&
                 !std::is_enum_v<std::decay_t<decltype(v)>> && (
                    requires { *v; bool(v); } ||
                    requires { v.first; v.second; } ||
                    requires { v.begin(); v.end(); v.size(); } ||
                    requires { v.GetCheckSum(); }
                ))
    {
        if constexpr (requires { *v; bool(v); }) {
            if (v)
                CheckSumCombine(sum, *v);

        } else if constexpr (requires { v.first; v.second; }) {
            CheckSumCombine(sum, v.first);
            CheckSumCombine(sum, v.second);
            CheckSumCombine(sum, 2u);

        } else if constexpr (requires { v.begin(); v.end(); v.size(); }) {
            for (const auto& t : v)
                CheckSumCombine(sum, t);
            CheckSumCombine(sum, v.size());

        } else if constexpr (requires { v.GetCheckSum(); }) {
            CheckSumCombine(sum, v.GetCheckSum());
        }
    }

    // tuples
    template <typename... Ts>
    constexpr void CheckSumCombine(uint32_t& sum, const std::tuple<Ts...>& ts)
    {
        const auto combine_t = [&sum](const auto& t)
        { CheckSumCombine(sum, t); };

        const auto combine_ts = [&combine_t](const auto&... ts)
        { (combine_t(ts), ...); };

        std::apply(combine_ts, ts);

        CheckSumCombine(sum, sizeof...(Ts));
    }

    template <typename Combinable> requires requires(const Combinable& c, uint32_t& i) { CheckSumCombine(i, c); }
    [[nodiscard]] constexpr uint32_t GetCheckSum(const Combinable& c)
        noexcept(noexcept(CheckSumCombine(std::declval<uint32_t&>(), c)))
    {
        uint32_t retval{0};
        CheckSumCombine(retval, c);
        return retval;
    }
}


#endif
