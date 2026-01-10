#ifndef _CheckSums_h_
#define _CheckSums_h_

#include "Export.h"

#include <cmath>
#include <cstdint>
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
        sum += static_cast<uint32_t>(sv.size());
        sum %= CHECKSUM_MODULUS;
    }
    constexpr void CheckSumCombine(uint32_t& sum, const char* s) noexcept
    { CheckSumCombine(sum, std::string_view{s}); }

    // classes that have GetCheckSum methods
    template <typename C> //requires (requires(C c) { c.GetCheckSum(); })
    void CheckSumCombine(uint32_t& sum, const C& c,
                         decltype(std::declval<C>().GetCheckSum())* = nullptr) noexcept(noexcept(c.GetCheckSum()))
    {
        sum += c.GetCheckSum();
        sum %= CHECKSUM_MODULUS;
    }

    // enums
    template <typename T> requires (std::is_enum_v<T>)
    constexpr void CheckSumCombine(uint32_t& sum, T t) noexcept
    { CheckSumCombine(sum, static_cast<int>(t) + 10); }

    // pointer types
    template <typename T>
    constexpr void CheckSumCombine(uint32_t& sum, const T* p) noexcept
    {
        if (p)
            CheckSumCombine(sum, *p);
    }
    template <typename T>
    void CheckSumCombine(uint32_t& sum, const typename std::shared_ptr<T>& p)
    {
        if (p)
            CheckSumCombine(sum, *p);
    }
    template <typename T>
    constexpr void CheckSumCombine(uint32_t& sum, const typename std::unique_ptr<T>& p)
    {
        if (p)
            CheckSumCombine(sum, *p);
    }

    // pairs (including map value types)
    template <typename C, typename D>
    constexpr void CheckSumCombine(uint32_t& sum, const std::pair<C, D>& p)
    {
        CheckSumCombine(sum, p.first);
        CheckSumCombine(sum, p.second);
    }

    // iterable containers
    template <typename C> //requires (requires (C c) { c.begin(); c.end(); })
    void CheckSumCombine(uint32_t& sum, const C& c,
                         decltype(std::declval<C>().begin())* = nullptr,
                         decltype(std::declval<C>().end())* = nullptr)
    {
        for (const auto& t : c)
            CheckSumCombine(sum, t);
        sum += static_cast<uint32_t>(c.size());
        sum %= CHECKSUM_MODULUS;
    }
}


#endif
