#ifndef _CheckSums_h_
#define _CheckSums_h_

#include "Export.h"

#include <cmath>
#include <limits>
#include <limits.h>
#include <memory>
#include <utility>
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

    constexpr auto abs(const auto& i) noexcept {
        using I_t = std::decay_t<decltype(i)>;
        if constexpr (std::is_floating_point_v<I_t>)
            return i >= 0 ? i : -i;
        else if constexpr (std::is_unsigned_v<I_t>)
            return i;
        else if (std::is_constant_evaluated())
            return cmp_less_equal(I_t{0}, i) ? i : -i;
        else
            return std::abs(i);
    }
    static_assert(!abs(false));
    static_assert(abs(true));
    static_assert(abs(-1) == 1);
    static_assert(abs(0) == 0);
    static_assert(abs(1) == 1);

    // ensure type fits into CheckSum output
    template <typename T> requires(std::is_integral_v<std::decay_t<T>>)
    consteval auto nlm() noexcept { return std::numeric_limits<std::decay_t<T>>::max(); }
    static_assert(cmp_less_equal(nlm<uint32_t>(), nlm<uint64_t>()));
    static_assert(cmp_less_equal(nlm<int16_t>(), nlm<uint16_t>()));

    template <typename T> requires(std::is_enum_v<std::decay_t<T>>)
    consteval auto nlm() noexcept { return nlm<std::underlying_type_t<T>>(); }

    template <typename T> requires(std::is_integral_v<std::decay_t<T>> || std::is_enum_v<std::decay_t<T>>)
    consteval bool fits_in_uint32t() noexcept { return cmp_less_equal(nlm<T>(), nlm<uint32_t>()); }
    namespace Testing {
        static_assert(fits_in_uint32t<bool>());
        static_assert(fits_in_uint32t<int16_t>());
        static_assert(fits_in_uint32t<uint32_t>());
        static_assert(!fits_in_uint32t<int64_t>());
        static_assert(!fits_in_uint32t<uint64_t>());
        enum class test_enum : int16_t { ZERO, ONE, TWO };
        static_assert(fits_in_uint32t<test_enum>());
    }

    inline constexpr uint32_t CHECKSUM_MODULUS = 10000000U;    // reasonably big number that should be well below UINT_MAX, which is ~4.29x10^9 for 32 bit unsigned int
    static_assert(CHECKSUM_MODULUS < UINT_MAX/4);

    // float types
    inline constexpr bool csc_double = noexcept(noexcept(std::log10(std::abs(43.0))));
    FO_COMMON_API void CheckSumCombine(uint32_t& sum, double t) noexcept(csc_double);

    inline constexpr bool csc_float = noexcept(noexcept(std::log10(std::abs(43.0f))));
    FO_COMMON_API void CheckSumCombine(uint32_t& sum, float t) noexcept(csc_float);


    // integeral types
    template <typename T> requires (std::is_integral_v<T>)
    constexpr void CheckSumCombine(uint32_t& sum, T t) noexcept
    {
        static_assert(noexcept(sum + static_cast<uint32_t>(t)));
        static_assert(noexcept(sum % CHECKSUM_MODULUS));
        if constexpr (!fits_in_uint32t<T>())
            t %= CHECKSUM_MODULUS;
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

    // classes that have GetCheckSum methods
    template <typename ClassWithGetCheckSum> requires requires(const ClassWithGetCheckSum& c) { c.GetCheckSum(); }
    constexpr void CheckSumCombine(uint32_t& sum, const ClassWithGetCheckSum& c)
    { CheckSumCombine(sum, c.GetCheckSum()); }

    // enums
    template <typename EnumT> requires (std::is_enum_v<EnumT>)
    constexpr void CheckSumCombine(uint32_t& sum, EnumT t) noexcept
    { CheckSumCombine(sum, static_cast<int>(t) + 10); }

    // pointer types
    template <typename PointerT> requires requires(const PointerT& ptr) { *ptr; }
    constexpr void CheckSumCombine(uint32_t& sum, const PointerT& ptr)
    { if (ptr) CheckSumCombine(sum, *ptr); }

    // pairs (including map value types)
    template <typename Combinable1, typename Combinable2>
      /*requires requires(const Combinable1& c1, const Combinable2& c2, uint32_t& i)
        { CheckSumCombine(i, c1); CheckSumCombine(i, c2); }*/
    constexpr void CheckSumCombine(uint32_t& sum, const std::pair<Combinable1, Combinable2>& p)
        //noexcept(noexcept(CheckSumCombine(sum, p.first)) && noexcept(CheckSumCombine(sum, p.second)))
    {
        CheckSumCombine(sum, p.first);
        CheckSumCombine(sum, p.second);
        CheckSumCombine(sum, 2u);
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
    template <typename ContainerOfCombinable> requires (
        requires(const ContainerOfCombinable& c, uint32_t& i) { c.begin(); c.end(); c.size(); CheckSumCombine(i, *c.begin()); } &&
        !std::is_same_v<std::string, std::decay_t<ContainerOfCombinable>> &&
        !std::is_same_v<std::string_view, std::decay_t<ContainerOfCombinable>>)
    constexpr void CheckSumCombine(uint32_t& sum, const ContainerOfCombinable& c)
    {
        for (const auto& t : c)
            CheckSumCombine(sum, t);
        CheckSumCombine(sum, c.size());
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
