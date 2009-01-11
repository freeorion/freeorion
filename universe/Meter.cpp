#include "Meter.h"

#include <algorithm>
#include <boost/lexical_cast.hpp>

const double Meter::METER_MIN = 0.0;
const double Meter::METER_MAX = 100.0;

Meter::Meter() :
    m_current(METER_MIN),
    m_max(METER_MIN),
    m_initial_current(METER_MIN),
    m_initial_max(METER_MIN),
    m_previous_current(METER_MIN),
    m_previous_max(METER_MIN)
{}

Meter::Meter(double current, double max) :
    m_current(current),
    m_max(max),
    m_initial_current(METER_MIN),
    m_initial_max(METER_MIN),
    m_previous_current(METER_MIN),
    m_previous_max(METER_MIN)
{}

Meter::Meter(double current, double max, double initial_current, double initial_max, double previous_current, double previous_max) :
    m_current(current),
    m_max(max),
    m_initial_current(initial_current),
    m_initial_max(initial_max),
    m_previous_current(previous_current),
    m_previous_max(previous_max)
{}

double Meter::Current() const
{
    return m_current;
}

double Meter::Max() const
{
    return m_max;
}

double Meter::InitialCurrent() const
{
    return m_initial_current;
}

double Meter::InitialMax() const
{
    return m_initial_max;
}

double Meter::PreviousCurrent() const
{
    return m_previous_current;
}

double Meter::PreviousMax() const
{
    return m_previous_max;
}

std::string Meter::Dump() const
{
    using boost::lexical_cast;
    return "Cur: " + lexical_cast<std::string>(m_current) + "/" + lexical_cast<std::string>(m_max) +
           " Initial Cur: " + lexical_cast<std::string>(m_initial_current) + "/" + lexical_cast<std::string>(m_initial_max) +
           " Prev: " + lexical_cast<std::string>(m_previous_current) + "/" + lexical_cast<std::string>(m_previous_max);
}

void Meter::BackPropegate()
{
    m_previous_current =    m_initial_current;
    m_previous_max =        m_initial_max;
    m_initial_current =     m_current;
    m_initial_max =         m_max;
}

void Meter::ResetMax()
{
    m_max = METER_MIN;
}

void Meter::SetCurrent(double current)
{
    m_current = current;    // allows current above m_max or below 0.  Meters are clamped later.
}

void Meter::SetMax(double max)
{
    m_max = max;            // allows max above 100 or below 0.  Meters are clamped later.
}

void Meter::AdjustCurrent(double adjustment)
{
    SetCurrent(m_current + adjustment);
}

void Meter::AdjustMax(double max)
{
    SetMax(m_max + max);
}

void Meter::Clamp()
{
    m_max = std::max(METER_MIN, std::min(m_max, METER_MAX));
    m_current = std::max(METER_MIN, std::min(m_current, m_max));
}

void Meter::Reset()
{
    m_current = METER_MIN;
    m_max = METER_MIN;
    m_initial_current = METER_MIN;
    m_initial_max = METER_MIN;
    m_previous_current = METER_MIN;
    m_previous_max = METER_MIN;
}

void Meter::Set(double current, double max, double initial_current, double initial_max, double previous_current, double previous_max)
{
    m_current = current;
    m_max = max;
    m_initial_current = initial_current;
    m_initial_max = initial_max;
    m_previous_current = previous_current;
    m_previous_max = previous_max;
}

void Meter::Set(double current, double max)
{
    m_current = current;
    m_max = max;
}
