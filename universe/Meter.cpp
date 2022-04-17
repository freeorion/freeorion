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
    // the biggest result of ToChars should be like "-65535.99" or 9 chars per number
    *result_ptr = ' ';
    *++result_ptr = 'I';
    *++result_ptr = 'n';
    *++result_ptr = 'i';
    *++result_ptr = 't';
    *++result_ptr = ':';
    *++result_ptr = ' ';
    ToChars(result_ptr + 1, FromInt(init));

    return buffer;
}

void Meter::ClampCurrentToRange(float min, float max) noexcept
{ cur = std::max(std::min(cur, FromFloat(max)), FromFloat(min)); }

template <>
void Meter::serialize(boost::archive::xml_iarchive& ar, const unsigned int version)
{
    if (version < 2) {
        float c = 0.0f, i = 0.0f;
        ar  & boost::serialization::make_nvp("c", c)
            & boost::serialization::make_nvp("i", i);
        cur = FromFloat(c);
        init = FromFloat(i);

    } else {
        std::string buffer;
        ar & boost::serialization::make_nvp("m", buffer);

#if defined(__cpp_lib_to_chars)
        auto buffer_end = buffer.data() + buffer.size();
        auto [ptr, ec] = std::from_chars(buffer.data(), buffer_end, cur);
        if (ec == std::errc())
            std::from_chars(ptr, buffer_end, init);
#else
        auto count = sscanf(buffer.data(), "%d %d", &cur, &init);
#endif
    }
}


#include "../util/Logger.h"

template <>
void Meter::serialize(boost::archive::xml_oarchive& ar, const unsigned int version)
{
#if defined(__cpp_lib_to_chars)
    std::array<char, 32> buffer{};
    auto buffer_end = buffer.data() + buffer.size();
    auto result_ptr = std::to_chars(buffer.data(), buffer_end, cur).ptr;
    *result_ptr++ = ' ';
    std::to_chars(result_ptr, buffer_end, init);
    ar << boost::serialization::make_nvp("m", std::string{buffer.data()});

#else
    std::string buffer;
    buffer.append(std::to_string(cur)).append(" ").append(std::to_string(init));
    ar & boost::serialization::make_nvp("m", *buffer.data());
#endif

}
