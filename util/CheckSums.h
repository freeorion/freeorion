#ifndef _CheckSums_h_
#define _CheckSums_h_

#include "Export.h"

#include <cmath>
#include <limits.h>
#include <memory>
#include <utility>
#include <string_view>

namespace CheckSums {
    inline constexpr uint32_t CHECKSUM_MODULUS = 10000000U;    // reasonably big number that should be well below UINT_MAX, which is ~4.29x10^9 for 32 bit unsigned int
    static_assert(CHECKSUM_MODULUS < UINT_MAX/4);

    inline constexpr bool csc_double = noexcept(noexcept(std::log10(std::abs(43.0))));
    FO_COMMON_API void CheckSumCombine(uint32_t& sum, double t) noexcept(csc_double);

    inline constexpr bool csc_float = noexcept(noexcept(std::log10(std::abs(43.0f))));
    FO_COMMON_API void CheckSumCombine(uint32_t& sum, float t) noexcept(csc_float);

    // integeral types
    template <typename T> requires (std::is_signed_v<T>)
    constexpr void CheckSumCombine(uint32_t& sum, T t) noexcept
    {
        sum += static_cast<uint32_t>(t >= 0 ? t : -t);
        sum %= CHECKSUM_MODULUS;
    }
    template <typename T> requires (std::is_unsigned_v<T>)
    constexpr void CheckSumCombine(uint32_t& sum, T t) noexcept
    {
        static_assert(noexcept(sum + static_cast<uint32_t>(t)) && noexcept(sum % CHECKSUM_MODULUS));
        sum += static_cast<uint32_t>(t);
        sum %= CHECKSUM_MODULUS;
    }
    template <>
    constexpr void CheckSumCombine(uint32_t& sum, signed char t) noexcept
    {
        static_assert(noexcept(sum + static_cast<uint32_t>(static_cast<unsigned char>(t))));
        static_assert(noexcept(sum % CHECKSUM_MODULUS));
        sum += static_cast<uint32_t>(static_cast<unsigned char>(t));
        sum %= CHECKSUM_MODULUS;
    }
    template <>
    constexpr void CheckSumCombine(uint32_t& sum, char t) noexcept
    {
        if constexpr (std::is_signed_v<char>)
            CheckSumCombine<signed char>(sum, t);
        else
            CheckSumCombine<unsigned char>(sum, t);
    }


    // strings
    constexpr void CheckSumCombine(uint32_t& sum, std::string_view sv) noexcept {
        for (auto t : sv)
            CheckSumCombine(sum, t);
        CheckSumCombine(sum, sv.size());
    }
    constexpr void CheckSumCombine(uint32_t& sum, const char* s) noexcept(noexcept(std::string_view{""}))
    { CheckSumCombine(sum, std::string_view{s}); }

    // classes that have GetCheckSum methods
    constexpr void CheckSumCombine(uint32_t& sum, const auto& c) noexcept(noexcept(c.GetCheckSum()))
        requires(requires { c.GetCheckSum(); })
    {
        sum += c.GetCheckSum();
        sum %= CHECKSUM_MODULUS;
    }

    // enums
    template <typename T> requires (std::is_enum_v<T>)
    constexpr void CheckSumCombine(uint32_t& sum, T t) noexcept
    { CheckSumCombine(sum, static_cast<int>(t) + 10); }

    // pointer types
    constexpr void CheckSumCombine(uint32_t& sum, const auto& ptr) noexcept
        requires(requires { *ptr; ptr.get(); })
    { if (ptr) CheckSumCombine(sum, *ptr); }

    // pairs (including map value types)
    template <typename C, typename D>
    constexpr void CheckSumCombine(uint32_t& sum, const std::pair<C, D>& p)
    {
        CheckSumCombine(sum, p.first);
        CheckSumCombine(sum, p.second);
        CheckSumCombine(sum, 2);
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

    // iterable containers
    constexpr void CheckSumCombine(uint32_t& sum, const auto& c)
        requires(requires { c.begin(); c.end(); c.size(); } &&
                 !std::is_same_v<std::string, std::decay_t<decltype(c)>>)
    {
        for (const auto& t : c)
            CheckSumCombine(sum, t);
        CheckSumCombine(sum, c.size());
    }

    [[nodiscard]] constexpr uint32_t GetCheckSum(const auto& c)
        noexcept(noexcept(CheckSumCombine(std::declval<uint32_t&>(), c)))
        requires(requires { CheckSumCombine(std::declval<uint32_t&>(), c); })
    {
        uint32_t retval{0};
        CheckSumCombine(retval, c);
        return retval;
    }
}


#endif
