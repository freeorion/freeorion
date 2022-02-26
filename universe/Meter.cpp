#include "Meter.h"

#include <algorithm>
#include <array>

#if __has_include(<charconv>)
#include <charconv>
#else
#include <stdio.h>
#endif


Meter::Meter(float current_value, float initial_value) :
    m_current_value(current_value),
    m_initial_value(initial_value)
{}

float Meter::Current() const
{ return m_current_value; }

float Meter::Initial() const
{ return m_initial_value; }

std::string Meter::Dump(unsigned short ntabs) const {
    std::array<std::string::value_type, 64> buffer{"Cur: "}; // rest should be nulls
#if defined(__cpp_lib_to_chars)
    auto ToChars = [buf_end{buffer.data() + buffer.size()}](char* buf_start, float num) -> char * {
        int precision = num < 10 ? 2 : 1;
        return std::to_chars(buf_start, buf_end, num, std::chars_format::fixed, precision).ptr;
    };
#else
    auto ToChars = [buf_end{buffer.data() + buffer.size()}](char* buf_start, float num) -> char * {
        auto count = sprintf(buf_start, num < 10 ? "%1.2f" : "%5.1f", num);
        return buf_start + std::max(0, count);
    };
#endif
    auto result_ptr = ToChars(buffer.data() + 5, m_current_value);
    // the biggest result of to_chars should be like "-65535.999" or 10 chars per number
    *result_ptr = ' ';
    *++result_ptr = 'I';
    *++result_ptr = 'n';
    *++result_ptr = 'i';
    *++result_ptr = 't';
    *++result_ptr = ':';
    *++result_ptr = ' ';
    ToChars(result_ptr + 1, m_initial_value);

    return buffer.data();
}

void Meter::SetCurrent(float current_value)
{ m_current_value = current_value; }

void Meter::Set(float current_value, float initial_value) {
    m_current_value = current_value;
    m_initial_value = initial_value;
}

void Meter::ResetCurrent()
{ m_current_value = DEFAULT_VALUE; } // initial unchanged

void Meter::Reset() {
    m_current_value = DEFAULT_VALUE;
    m_initial_value = DEFAULT_VALUE;
}

void Meter::AddToCurrent(float adjustment)
{ m_current_value += adjustment; }

void Meter::ClampCurrentToRange(float min/* = DEFAULT_VALUE*/, float max/* = LARGE_VALUE*/)
{ m_current_value = std::max(std::min(m_current_value, max), min); }

void Meter::BackPropagate()
{ m_initial_value = m_current_value; }
