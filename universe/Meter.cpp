#include "Meter.h"

#include "../util/Serialize.h"

#include <algorithm>
#include <array>

#if __has_include(<charconv>)
#include <charconv>
#else
#include <stdio.h>
#endif

std::array<std::string::value_type, 64> Meter::Dump(unsigned short ntabs) const noexcept {
    std::array<std::string::value_type, 64> buffer{"Cur: "}; // rest should be nulls
#if defined(__cpp_lib_to_chars)
    auto ToChars = [buf_end{buffer.data() + buffer.size()}](char* buf_start, float num) -> char * {
        int precision = num < 10 ? 2 : 1;
        return std::to_chars(buf_start, buf_end, num, std::chars_format::fixed, precision).ptr;
    };
#else
    auto ToChars = [buf_end{buffer.data() + buffer.size()}](char* buf_start, float num) -> char * {
        auto count = snprintf(buf_start, 10, num < 10 ? "%1.2f" : "%5.1f", num);
        return buf_start + std::max(0, count);
    };
#endif
    auto result_ptr = ToChars(buffer.data() + 5, FromInt(cur));
    // due to decimal precision of at most 2, the biggest result of to_chars
    // should be like "-65535.99" or 9 chars per number, if constrained by
    // LARGE_VALUE, but Meter can be initialized with larger values, so
    // a full 64-char array is used as the buffer and returned.
    static constexpr std::string_view init_label = " Init: ";
    std::copy_n(init_label.data(), init_label.size(), result_ptr);
    result_ptr += init_label.size();
    ToChars(result_ptr, FromInt(init));

    return buffer;
}

void Meter::ClampCurrentToRange(float min, float max) noexcept
{ cur = std::max(std::min(cur, FromFloat(max)), FromFloat(min)); }

namespace {
    template <typename T>
    constexpr T Pow(T base, T exp) {
        T retval = 1;
        while (exp--)
            retval *= base;
        return retval;
    }

    template <typename T, std::size_t N>
    constexpr std::size_t ArrSize(std::array<T, N>)
    { return N; }
}

Meter::ToCharsArrayT Meter::ToChars() const {
    static constexpr auto max_val = std::numeric_limits<int>::max();
    static_assert(max_val < Pow(10LL, 10LL)); // ensure serialized form of int can fit in 11 digits
    static_assert(max_val > Pow(10, 9));
    static constexpr auto digits_one_int = 1 + 10;
    static constexpr auto digits_meter = 2*digits_one_int + 1 + 1; // two numbers, one space, one padding to be safe
    static_assert(ArrSize(ToCharsArrayT()) == digits_meter);

    ToCharsArrayT buffer{};
    ToChars(buffer.data(), buffer.data() + buffer.size());
    return buffer;
}

size_t Meter::ToChars(char* buffer, char* buffer_end) const {
#if defined(__cpp_lib_to_chars)
    auto result_ptr = std::to_chars(buffer, buffer_end, cur).ptr;
    *result_ptr++ = ' ';
    result_ptr = std::to_chars(result_ptr, buffer_end, init).ptr;
    return std::distance(buffer, result_ptr);
#else
    std::size_t buffer_sz = std::distance(buffer, buffer_end);
    auto temp = std::to_string(cur);
    auto out_sz = temp.size();
    std::copy_n(temp.begin(), std::min(buffer_sz, out_sz), buffer);
    std::advance(buffer, temp.size());
    *buffer++ = ' ';
    out_sz += 1;
    buffer_sz = std::distance(buffer, buffer_end);
    temp = std::to_string(init);
    out_sz += temp.size();
    std::copy_n(temp.begin(), std::min(buffer_sz, temp.size()), buffer);
    return out_sz;
#endif
}

size_t Meter::SetFromChars(std::string_view chars) {
#if defined(__cpp_lib_to_chars)
    auto buffer_end = chars.data() + chars.size();
    auto result = std::from_chars(chars.data(), buffer_end, cur);
    if (result.ec == std::errc()) {
        ++result.ptr; // for ' ' separator
        result = std::from_chars(result.ptr, buffer_end, init);
    }
    return std::distance(chars.data(), result.ptr);
#else
    int chars_consumed = 0;
    sscanf(chars.data(), "%d %d%n", &cur, &init, &chars_consumed);
    return chars_consumed;
#endif
}

template <>
void Meter::serialize(boost::archive::xml_iarchive& ar, const unsigned int version)
{
    using namespace boost::serialization;
    using Archive_t = typename std::remove_reference_t<decltype(ar)>;
    static_assert(Archive_t::is_loading::value);
    if (version < 2) {
        float c = 0.0f, i = 0.0f;
        ar  & make_nvp("c", c)
            & make_nvp("i", i);
        cur = FromFloat(c);
        init = FromFloat(i);

    } else {
        std::string buffer;
        ar >> make_nvp("m", buffer);
        SetFromChars(buffer);
    }
}

template <>
void Meter::serialize(boost::archive::xml_oarchive& ar, const unsigned int version)
{
    using namespace boost::serialization;
    using Archive_t = typename std::remove_reference_t<decltype(ar)>;
    static_assert(Archive_t::is_saving::value);
    std::string s{ToChars().data()};
    ar << make_nvp("m", s);
}
