#include "PopCenter.h"

#include "../util/AppInterface.h"
#include "XMLDoc.h"

#include <boost/lexical_cast.hpp>

#include <stdexcept>

using boost::lexical_cast;


PopCenter::PopCenter() : 
    m_pop(),
    m_health(),
    m_growth(0.0),
    m_race(-1),
    m_available_food(0.0)
{
}
   
PopCenter::PopCenter(double max_pop) : 
    m_pop(1.0, max_pop),
    m_health(0.0, 0.0),
    m_growth(0.0),
    m_race(-1),
    m_available_food(0.0)
{
}
   
PopCenter::PopCenter(double max_pop, int race) : 
    m_pop(1.0, max_pop),
    m_health(0.0, 0.0),
    m_growth(0.0),
    m_race(race),
    m_available_food(0.0)
{
}
   
PopCenter::PopCenter(const GG::XMLElement& elem)
{
    if (elem.Tag() != "PopCenter")
        throw std::invalid_argument("Attempted to construct a PopCenter from an XMLElement that had a tag other than \"PopCenter\"");

    try {
        m_pop = Meter(elem.Child("m_pop").Child("Meter"));
        m_health = Meter(elem.Child("m_health").Child("Meter"));
        m_growth = lexical_cast<double>(elem.Child("m_growth").Text());
        m_race = lexical_cast<int>(elem.Child("m_race").Text());
        m_available_food = lexical_cast<double>(elem.Child("m_available_food").Text());
    } catch (const boost::bad_lexical_cast& e) {
        Logger().debugStream() << "Caught boost::bad_lexical_cast in PopCenter::PopCenter(); bad XMLElement was:";
        std::stringstream osstream;
        elem.WriteElement(osstream);
        Logger().debugStream() << "\n" << osstream.str();
        throw;
    }
}

PopCenter::~PopCenter()
{
}

double PopCenter::Inhabitants() const
{
    double retval = 0.0;
    // TODO
    return retval;
}

PopCenter::DensityType PopCenter::PopDensity() const
{
    DensityType retval = OUTPOST;
    // TODO
    return retval;
}

GG::XMLElement PopCenter::XMLEncode(UniverseObject::Visibility vis) const
{
    // partial encode version.  PopCenter is always fully visible
    using GG::XMLElement;
    using boost::lexical_cast;

    XMLElement element("PopCenter");
    element.AppendChild(XMLElement("m_pop", m_pop.XMLEncode()));
    element.AppendChild(XMLElement("m_health", m_health.XMLEncode()));
    element.AppendChild(XMLElement("m_growth", lexical_cast<std::string>(m_growth)));
    element.AppendChild(XMLElement("m_race", lexical_cast<std::string>(m_race)));
    element.AppendChild(XMLElement("m_available_food", lexical_cast<std::string>(m_available_food)));

    return element;
}

double PopCenter::AdjustPop(double pop)
{
    double retval = 0.0;
    double new_val = m_pop.Current() + pop;
    m_pop.SetCurrent(new_val);
    if (new_val < Meter::METER_MIN) {
	retval = new_val;
    } else if (m_pop.Max() < new_val) {
	retval = new_val - m_pop.Max();
    }
    return retval;
}

void PopCenter::SetEnvHealthMod(double env_health_mod)
{
    env_health_mod = std::max(Meter::METER_MIN, std::min(env_health_mod, Meter::METER_MAX));
    m_health.SetMax(env_health_mod);
    m_health.SetCurrent(env_health_mod);
}

void PopCenter::MovementPhase()
{
}

double PopCenter::FuturePopGrowth() const
{
    return std::min(FuturePopGrowthMax(), AvailableFood() - PopPoints());
}

double PopCenter::FuturePopGrowthMax() const
{
    if (20.0 < m_health.Current()) {
	return m_pop.Current() * (((m_pop.Max() + 1.0) - m_pop.Current()) / (m_pop.Max() + 1.0)) * (m_health.Current() - 20.0) * 0.01;
    } else if (m_health.Current() == 20.0) {
	return 0.0;
    } else { // m_health.Current() < 20.0
	return m_pop.Current() * (m_health.Current() - 20.0) * 0.01;
    }
}

void PopCenter::PopGrowthProductionResearchPhase( )
{
    m_pop.SetCurrent(m_pop.Current() + FuturePopGrowth());
}



