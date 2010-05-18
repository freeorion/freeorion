#include "Meter.h"

#include <algorithm>
#include <boost/lexical_cast.hpp>

const double Meter::DEFAULT_VALUE = 0.0;
const double Meter::LARGE_VALUE = static_cast<double>(2 << 15);
const double Meter::INVALID_VALUE = -LARGE_VALUE;

Meter::Meter() :
    m_current_value(DEFAULT_VALUE),
    m_initial_value(DEFAULT_VALUE)
{}

Meter::Meter(double current_value) :
    m_current_value(current_value),
    m_initial_value(DEFAULT_VALUE)
{}

Meter::Meter(double current_value, double initial_value) :
    m_current_value(current_value),
    m_initial_value(initial_value)
{}

double Meter::Current() const { return m_current_value; }
double Meter::Initial() const { return m_initial_value; }

std::string Meter::Dump() const {
    using boost::lexical_cast;
    return "Cur: " + lexical_cast<std::string>(m_current_value) +
           " Initial: " + lexical_cast<std::string>(m_initial_value);
}

void Meter::SetCurrent(double current_value) { m_current_value = current_value; }

void Meter::Set(double current_value, double initial_value) {
    m_current_value = current_value;
    m_initial_value = initial_value;
}

void Meter::ResetCurrent() {
    m_current_value = DEFAULT_VALUE;
    // initial unchanged
}

void Meter::Reset() {
    m_current_value = DEFAULT_VALUE;
    m_initial_value = DEFAULT_VALUE;
}

void Meter::AddToCurrent(double adjustment)
{ m_current_value += adjustment; }

void Meter::ClampCurrentToRange(double min/* = DEFAULT_VALUE*/, double max/* = LARGE_VALUE*/) {
    m_current_value = std::max(std::min(m_current_value, max), min);
}

void Meter::BackPropegate() {
    m_initial_value = m_current_value;
}
