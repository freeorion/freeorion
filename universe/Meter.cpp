#include "Meter.h"

#include "../util/MultiplayerCommon.h"

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

double Meter::DeltaCurrent() const
{
    return m_initial_current - m_previous_current;
}

double Meter::DeltaMax() const
{
    return m_initial_max - m_previous_max;
}

void Meter::ResetMax()
{
    m_max = METER_MIN;
}

void Meter::SetCurrent(double current)
{
    m_current = std::max(METER_MIN, std::min(current, METER_MAX));
}

void Meter::SetMax(double max)
{
    m_max = std::max(METER_MIN, std::min(max, METER_MAX));
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
    m_current = std::max(METER_MIN, std::min(m_current, m_max));
}
