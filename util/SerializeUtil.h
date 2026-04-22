#include <algorithm>
#include <array>
#if __has_include(<charconv>)
# include <charconv>
#else
# include <cstdio>
#endif
#include <concepts>
#include <string>
#include <string_view>
#include <type_traits>

#include "../universe/ConstantsFwd.h"

#if !defined(CONSTEXPR_STRING)
#  if defined(__cpp_lib_constexpr_string) && ((!defined(__GNUC__) || (__GNUC__ > 11))) && ((!defined(_MSC_VER) || (_MSC_VER >= 1934)))
#    define CONSTEXPR_STRING constexpr
#  else
#    define CONSTEXPR_STRING
#  endif
#endif

namespace {
    // <concepts> library not fully implemented in XCode 13.2
    template <class T>
    concept integral = std::is_integral_v<T>;

    template <integral T>
    consteval const auto* GetFormatString() {
        if constexpr (std::is_same_v<T, unsigned int>)
            return "%u%n";
        else if constexpr (std::is_same_v<T, int>)
            return "%d%n";
        else
            static_assert(sizeof(T) == 0); // unsupported type
    }

    constexpr bool have_to_chars_lib =
#if defined(__cpp_lib_to_chars)
        true;
#else
        false;
#endif

#if defined(__cpp_lib_to_chars) && defined(__cpp_lib_constexpr_charconv)
    constexpr
#endif
        std::size_t ToChars(integral auto num, char* buffer, char* buffer_end) {
        if constexpr (have_to_chars_lib) {
            const auto result_ptr = std::to_chars(buffer, buffer_end, num).ptr;
            return static_cast<std::size_t>(std::distance(buffer, result_ptr));
        } else {
            std::size_t buffer_sz = std::distance(buffer, buffer_end);
            auto temp = std::to_string(num);
            auto out_sz = std::min(buffer_sz, temp.size());
            std::copy_n(temp.begin(), out_sz, buffer);
            return out_sz;
        }
    }

    // returns { next unconsumed char*, true/false did the parse succeed }
    // parsed value returned in \a val_out
#if defined(__cpp_lib_to_chars) && defined(__cpp_lib_constexpr_charconv)
    constexpr
#endif
    std::pair<const char*, bool> FromChars(const char* start, const char* end, integral auto& val_out) {
        if constexpr (have_to_chars_lib) {
            const auto result = std::from_chars(start, end, val_out);
            return {result.ptr, result.ec == std::errc() && result.ptr != start};

        } else {
            const auto rng_sz = std::distance(start, end);
            if (rng_sz < 1)
                return {start, false};

            std::array<char, 64> null_terminated_buffer{};

            // digits10 gives how many digits are guaranteed to be storable.
            // For char, digits10 is 2, and not 3 since 257+ is not possible to store in a char.
            // To get space needed to store full range of type, add:
            //   + 1 for potential sign char
            //   + 1 for the next digit above digits that can be stored losslessly
            //   + 1 for null terminator
            // based on that, verify buffer is big enough for any potential char representation
            // of values of type being parsed
            using val_out_t = std::decay_t<decltype(val_out)>;
            static constexpr std::size_t needed_chars_to_encode_all_values = std::numeric_limits<val_out_t>::digits10 + 3;
            static_assert(needed_chars_to_encode_all_values < null_terminated_buffer.size());

            // copy data into local buffer so it can be guaranteed null terminated
            const std::size_t copy_count = std::min(static_cast<std::size_t>(rng_sz), null_terminated_buffer.size() - 1);
            std::copy_n(start, copy_count, null_terminated_buffer.data());
            null_terminated_buffer.back() = 0;

            int chars_consumed = 0;
            constexpr auto val_format_str = GetFormatString<val_out_t>();
            const auto matched = sscanf(null_terminated_buffer.data(), val_format_str, &val_out, &chars_consumed);
            if (matched > 0)
                return {start + chars_consumed, chars_consumed > 0};
            else
                return {start, false};
        }
    }

#if defined(__cpp_lib_to_chars) && defined(__cpp_lib_constexpr_charconv)
    constexpr
#endif
    auto FromChars(std::string_view str, integral auto& val_out) { return FromChars(str.data(), str.data() + str.size(), val_out); };

    constexpr int int_max = std::numeric_limits<int>::max();
    constexpr uint8_t int_digits = 11; // digits in base 11 of -2147483648 = -2^31
    constexpr std::size_t ten_pow_int_digits = [](int exp) { std::size_t retval = 1; while (exp--) retval *= 10; return retval; }(int_digits + 1);
    static_assert(ten_pow_int_digits > static_cast<std::size_t>(int_max)); // biggest possible int should fit in buffer

    inline std::string ToString(const auto& data)
        requires requires { data.size(); } && std::is_same_v<int, std::decay_t<decltype(*data.begin())>>
    {
        std::string retval;

        try {
            retval.reserve(data.size() * (int_digits + 1) + int_digits + 2); // space for count and all values and gaps
        } catch(...) {}

        retval.append(std::to_string(data.size()));

        for (const auto& v : data)
            retval.append(" ").append(std::to_string(v));

        return retval;
    }

    inline void FillIntContainer(auto& container, std::string_view buffer)
        requires requires { container.push_back(1); } || requires { container.insert(1); }
    {
        if (buffer.empty())
            return;

        const auto* const buffer_end = buffer.data() + buffer.size();

        unsigned int count = 0;
        auto [next, success] = FromChars(buffer.data(), buffer_end, count);
        if (!success)
            return;

        if constexpr (requires { container.reserve(count); }) {
            try {
                container.reserve(count);
            } catch (...) {}
        }

        const auto get_int_from_chars = [buffer_end](const char* next, const int default_val) -> std::tuple<int, bool, const char*> {
            // safety checks
            if (!next || !buffer_end)
                return {default_val, false, next};

            // skip whitespace
            while (next != buffer_end && *next == ' ')
                ++next;

            // safety check for end of buffer
            if (next == buffer_end)
                return {default_val, false, next};

            // parse string to int
            int result = default_val;
            auto [next_out, success] = FromChars(next, buffer_end, result);
            return {result, success, next_out};
        };


        for (std::size_t idx = 0; idx < static_cast<std::size_t>(count) && next != buffer_end; ++idx) {
            int num = 0;

            std::tie(num, success, next) = get_int_from_chars(next, INVALID_OBJECT_ID);
            if (!success)
                break;

            if constexpr (requires { container.push_back(num); })
                container.push_back(num);
            else if constexpr (requires { container.insert(num); })
                container.insert(num);
        }
    }
}
