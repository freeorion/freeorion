#include "Meter.h"

namespace {
    bool temp_header_bool = RecordHeaderFile(MeterRevision());
    bool temp_source_bool = RecordSourceFile("$RCSfile$", "$Revision$");
}

const double Meter::METER_MIN = 0.0;
const double Meter::METER_MAX = 100.0;

Meter::Meter() :
    m_current(METER_MIN),
    m_max(METER_MIN)
{
}

Meter::Meter(double current, double max) :
    m_current(current),
    m_max(max)
{
}

Meter::Meter(const GG::XMLElement& elem)
{
    if (elem.Tag().find("Meter") == std::string::npos )
        throw std::invalid_argument("Attempted to construct a Meter from an XMLElement that had a tag other than \"Meter\"");

    m_current = boost::lexical_cast<double>(elem.Child("m_current").Text());
    m_max = boost::lexical_cast<double>(elem.Child("m_max").Text());
}

double Meter::Current() const
{
    return m_current;
}

double Meter::Max() const
{
    return m_max;
}

GG::XMLElement Meter::XMLEncode(int empire_id/* = Universe::ALL_EMPIRES*/) const
{
    GG::XMLElement retval("Meter");
    retval.AppendChild(GG::XMLElement("m_current", boost::lexical_cast<std::string>(m_current)));
    retval.AppendChild(GG::XMLElement("m_max", boost::lexical_cast<std::string>(m_max)));
    return retval;
}

void Meter::ResetMax()
{
    m_max = METER_MIN;
}

void Meter::SetCurrent(double current)
{
    m_current = std::max(METER_MIN, std::min(current, m_max));
}

void Meter::SetMax(double max)
{
    m_max = std::max(METER_MIN, std::min(max, METER_MAX));
}

void Meter::AdjustCurrent(double current)
{
    SetCurrent(m_current + current);
}

void Meter::AdjustMax(double max)
{
    SetMax(m_max + max);
}
