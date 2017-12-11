#include "Meter.h"

#include <algorithm>
#include <sstream>

const float Meter::LARGE_VALUE = static_cast<float>(2 << 15);
const float Meter::INVALID_VALUE = -LARGE_VALUE;

Meter::Meter()
{}

Meter::Meter(float current_value) :
    m_current_value(current_value)
{}

Meter::Meter(float current_value, float initial_value) :
    m_current_value(current_value),
    m_initial_value(initial_value)
{}

float Meter::Current() const
{ return m_current_value; }

float Meter::Initial() const
{ return m_initial_value; }

std::string Meter::Dump(unsigned short ntabs) const {
    std::ostringstream strstm;
    strstm.precision(5);
    strstm << "Cur: " << m_current_value << " Init: " << m_initial_value;
    return strstm.str();
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
