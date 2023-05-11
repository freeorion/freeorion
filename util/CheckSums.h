#ifndef _CheckSums_h_
#define _CheckSums_h_

#include "Export.h"

#include <climits>
#include <cmath>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <typeinfo>
#include <vector>

namespace CheckSums {
    constexpr uint32_t CHECKSUM_MODULUS = 10000000U;    // reasonably big number that should be well below UINT_MAX, which is ~4.29x10^9 for 32 bit unsigned int
    static_assert(CHECKSUM_MODULUS < UINT_MAX/4);

    constexpr bool csc_double = noexcept(noexcept(std::log10(std::abs(43.0))));
    FO_COMMON_API void CheckSumCombine(uint32_t& sum, double t) noexcept(csc_double);

    constexpr bool csc_float = noexcept(noexcept(std::log10(std::abs(43.0f))));
    FO_COMMON_API void CheckSumCombine(uint32_t& sum, float t) noexcept(csc_float);

    FO_COMMON_API void CheckSumCombine(uint32_t& sum, const char* s);
    FO_COMMON_API void CheckSumCombine(uint32_t& sum, const std::string& c);

    // integeral types
    template <typename T> requires (std::is_signed_v<T>)
    void CheckSumCombine(uint32_t& sum, T t)
    {
        sum += static_cast<uint32_t>(std::abs(t));
        sum %= CHECKSUM_MODULUS;
    }
    template <typename T> requires (std::is_unsigned_v<T>)
    constexpr void CheckSumCombine(uint32_t& sum, T t) noexcept
    {
        static_assert(noexcept(sum + static_cast<uint32_t>(t)) && noexcept(sum % CHECKSUM_MODULUS));
        sum += static_cast<uint32_t>(t);
        sum %= CHECKSUM_MODULUS;
    }

    // classes that have GetCheckSum methods
    template <typename C> //requires (requires(C c) { c.GetCheckSum(); })
    void CheckSumCombine(uint32_t& sum, const C& c,
                         decltype(std::declval<C>().GetCheckSum())* = nullptr)
    {
        sum += c.GetCheckSum();
        sum %= CHECKSUM_MODULUS;
    }

    // enums
    template <typename T> requires (std::is_enum_v<T>)
    void CheckSumCombine(uint32_t& sum, T t)
    { CheckSumCombine(sum, static_cast<int>(t) + 10); }

    // pointer types
    template <typename T>
    void CheckSumCombine(uint32_t& sum, const T* p)
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
    void CheckSumCombine(uint32_t& sum, const typename std::unique_ptr<T>& p)
    {
        if (p)
            CheckSumCombine(sum, *p);
    }

    // pairs (including map value types)
    template <typename C, typename D>
    void CheckSumCombine(uint32_t& sum, const std::pair<C, D>& p)
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
