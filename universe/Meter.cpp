#include "Meter.h"

#include "../util/MultiplayerCommon.h"

namespace {
    bool temp_header_bool = RecordHeaderFile(MeterRevision());
    bool temp_source_bool = RecordSourceFile("$RCSfile$", "$Revision$");
}

const double Meter::METER_MIN = 0.0;
const double Meter::METER_MAX = 100.0;

Meter::Meter() :
    m_current(METER_MIN),
    m_max(METER_MIN),
    m_initial_current(METER_MIN),
    m_initial_max(METER_MIN),
    m_previous_current(METER_MIN),
    m_previous_max(METER_MIN)
{
}

Meter::Meter(double current, double max) :
    m_current(current),
    m_max(max),
    m_initial_current(METER_MIN),
    m_initial_max(METER_MIN),
    m_previous_current(METER_MIN),
    m_previous_max(METER_MIN)
{
}

Meter::Meter(const GG::XMLElement& elem)
{
    if (elem.Tag().find("Meter") == std::string::npos )
        throw std::invalid_argument("Attempted to construct a Meter from an XMLElement that had a tag other than \"Meter\"");

    m_current = boost::lexical_cast<double>(elem.Child("m_current").Text());
    m_max = boost::lexical_cast<double>(elem.Child("m_max").Text());
    m_initial_current = boost::lexical_cast<double>(elem.Child("m_initial_current").Text());
    m_initial_max = boost::lexical_cast<double>(elem.Child("m_initial_max").Text());
    m_previous_current = boost::lexical_cast<double>(elem.Child("m_previous_current").Text());
    m_previous_max = boost::lexical_cast<double>(elem.Child("m_previous_max").Text());
}

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

GG::XMLElement Meter::XMLEncode(int empire_id/* = Universe::ALL_EMPIRES*/) const
{
    GG::XMLElement retval("Meter");
    retval.AppendChild(GG::XMLElement("m_current", boost::lexical_cast<std::string>(m_current)));
    retval.AppendChild(GG::XMLElement("m_max", boost::lexical_cast<std::string>(m_max)));
    retval.AppendChild(GG::XMLElement("m_initial_current", boost::lexical_cast<std::string>(m_initial_current)));
    retval.AppendChild(GG::XMLElement("m_initial_max", boost::lexical_cast<std::string>(m_initial_max)));
    retval.AppendChild(GG::XMLElement("m_previous_current", boost::lexical_cast<std::string>(m_previous_current)));
    retval.AppendChild(GG::XMLElement("m_previous_max", boost::lexical_cast<std::string>(m_previous_max)));
    return retval;
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
