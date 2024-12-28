#include "Meter.h"

#include "../util/Serialize.h"

#include <algorithm>
#include <array>

#if !__has_include(<charconv>)
#include <cstdio>
#endif

namespace {
    // rounding checks
    constexpr Meter r1{65000.001f};
    constexpr Meter r2 = []() {
        Meter r2r = r1;
        r2r.AddToCurrent(-1.0f);
        return r2r;
    }();
    static_assert(r2.Current() == 64999.001f);
    constexpr Meter r3 = []() {
        Meter r3r = r2;
        r3r.AddToCurrent(-0.01f);
        return r3r;
    }();
    static_assert(r3.Current() == 64998.991f);
    constexpr Meter r4 = []() {
        Meter r4r = r3;
        r4r.AddToCurrent(40.009f);
        return r4r;
    }();
    static_assert(r4.Current() == 65039.0f);

    constexpr Meter q1{2.01f};
    constexpr Meter q2 = []() {
        Meter q2r = q1;
        q2r.AddToCurrent(-1.0f);
        return q2r;
    }();
    static_assert(q2.Current() == 1.01f);
    constexpr Meter q3 = []() {
        Meter q3r = q2;
        q3r.AddToCurrent(-1.0f);
        return q3r;
    }();
    static_assert(q3.Current() == 0.01f);
}

std::array<std::string::value_type, 64> Meter::Dump(uint8_t ntabs) const noexcept(dump_noexcept) {
    std::array<std::string::value_type, 64> buffer{"Cur: "}; // rest should be nulls
#if defined(__cpp_lib_to_chars)
    auto ToChars4Dump = [buf_end{buffer.data() + buffer.size()}](char* buf_start, float num)
        noexcept(have_noexcept_to_chars) -> char *
    {
        const int precision = num < 10 ? 2 : 1;
        return std::to_chars(buf_start, buf_end, num, std::chars_format::fixed, precision).ptr;
    };
#else
    auto ToChars4Dump = [buf_end{buffer.data() + buffer.size()}](char* buf_start, float num) -> char * {
        const auto count = snprintf(buf_start, 10, num < 10 ? "%1.2f" : "%5.1f", num);
        return buf_start + std::max(0, count);
    };
#endif
    auto result_ptr = ToChars4Dump(buffer.data() + 5, FromInt(cur));
    // due to decimal precision of at most 2, the biggest result of to_chars
    // should be like "-65535.99" or 9 chars per number, if constrained by
    // LARGE_VALUE, but Meter can be initialized with larger values, so
    // a full 64-char array is used as the buffer and returned.
    static constexpr std::string_view init_label = " Init: ";
    std::copy_n(init_label.data(), init_label.size(), result_ptr); // assuming noexcept since result_ptr should point into buffer and init_label is constexpr
    result_ptr += init_label.size();
    ToChars4Dump(result_ptr, FromInt(init));

    return buffer;
}

void Meter::ClampCurrentToRange(float min, float max) // no noexcept because using std::max and std::min in header cause symbol definintion problems on Windows
{ cur = std::max(std::min(cur, FromFloat(max)), FromFloat(min)); }

namespace {
    template <typename T>
    consteval T Pow(T base, T exp) {
        T retval = 1;
        while (exp--)
            retval *= base;
        return retval;
    }
}

Meter::ToCharsArrayT Meter::ToChars() const noexcept(have_noexcept_to_chars) {
    static constexpr auto max_val = std::numeric_limits<int>::max();
    static_assert(max_val < Pow(10LL, 10LL)); // ensure serialized form of int can fit in 11 digits
    static_assert(max_val > Pow(10, 9));
    static constexpr auto digits_one_int = 1 + 10;
    static constexpr auto digits_meter = 2*digits_one_int + 1 + 1; // two numbers, one space, one padding to be safe
    static_assert(std::size(ToCharsArrayT()) == digits_meter);

    ToCharsArrayT buffer{};
    ToChars(buffer.data(), buffer.data() + buffer.size());
    return buffer;
}

int Meter::ToChars(char* buffer, char* const buffer_end) const noexcept(have_noexcept_to_chars) {
#if defined(__cpp_lib_to_chars)
    auto result_ptr = std::to_chars(buffer, buffer_end, cur).ptr;
    *result_ptr++ = ' ';
    const auto result_ptr2 = std::to_chars(result_ptr, buffer_end, init).ptr;
    static_assert(noexcept(result_ptr2 - buffer));

    return result_ptr2 - buffer;
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

int Meter::SetFromChars(std::string_view chars) noexcept(have_noexcept_to_chars) {
#if defined(__cpp_lib_to_chars)
    const auto buffer_end = chars.data() + chars.size();
    auto result = std::from_chars(chars.data(), buffer_end, cur);
    if (result.ec == std::errc()) {
        ++result.ptr; // for ' ' separator
        result = std::from_chars(result.ptr, buffer_end, init);
    }
    return result.ptr - chars.data();

    static_assert(noexcept(result.ptr - chars.data()));
#else
    const std::string null_terminated_chars{chars};
    int chars_consumed = 0;
    std::sscanf(null_terminated_chars.data(), "%d %d%n", &cur, &init, &chars_consumed);
    return chars_consumed;
#endif

    static_assert(DEFAULT_INT == FromFloat(DEFAULT_VALUE));
    static_assert(LARGE_INT == FromFloat(LARGE_VALUE));
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
