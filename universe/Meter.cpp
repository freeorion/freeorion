#include "Meter.h"

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
    auto result_ptr = ToChars(buffer.data() + 5, cur);
    // the biggest result of to_chars should be like "-65535.99" or 9 chars per number
    *result_ptr = ' ';
    *++result_ptr = 'I';
    *++result_ptr = 'n';
    *++result_ptr = 'i';
    *++result_ptr = 't';
    *++result_ptr = ':';
    *++result_ptr = ' ';
    ToChars(result_ptr + 1, init);

    return buffer;
}

void Meter::ClampCurrentToRange(float min, float max) noexcept
{ cur = std::max(std::min(cur, max), min); }
