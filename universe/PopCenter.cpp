#include "PopCenter.h"

#include "../util/AppInterface.h"
#include "../util/DataTable.h"
#include "Planet.h"
#include "XMLDoc.h"

#include <boost/lexical_cast.hpp>

#include <stdexcept>

using boost::lexical_cast;

namespace {
    DataTableMap& PlanetDataTables()
    {
        static DataTableMap map;
        if (map.empty()) {
            LoadDataTables("default/planet_tables.txt", map);
        }
        return map;
    }

    double MaxPopModFromObject(const UniverseObject* object)
    {
        double retval = 0.0;
        if (const Planet* planet = dynamic_cast<const Planet*>(object)) {
            retval = PlanetDataTables()["PlanetMaxPop"][planet->Size()][planet->Environment()];
        }
        return retval;
    }

    double MaxHealthModFromObject(const UniverseObject* object)
    {
        double retval = 0.0;
        if (const Planet* planet = dynamic_cast<const Planet*>(object)) {
            retval = PlanetDataTables()["PlanetEnvHealthMod"][0][planet->Environment()];
        }
        return retval;
    }

    bool temp_header_bool = RecordHeaderFile(PopCenterRevision());
    bool temp_source_bool = RecordSourceFile("$RCSfile$", "$Revision$");
}

PopCenter::PopCenter(UniverseObject* object) : 
    m_pop(0.0, MaxPopModFromObject(object)),
    m_health(MaxHealthModFromObject(object), MaxHealthModFromObject(object)),
    m_growth(0.0),
    m_race(-1),
    m_available_food(0.0),
    m_object(object)
{
    assert(m_object);
}
   
PopCenter::PopCenter(int race, UniverseObject* object) : 
    m_pop(0.0, MaxPopModFromObject(object)),
    m_health(MaxHealthModFromObject(object), MaxHealthModFromObject(object)),
    m_growth(0.0),
    m_race(race),
    m_available_food(0.0),
    m_object(object)
{
    assert(m_object);
}
   
PopCenter::PopCenter(const GG::XMLElement& elem, UniverseObject* object) :
    m_object(object)
{
    assert(m_object);

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

void PopCenter::AdjustMaxMeters()
{
    // determine meter maxes; they should have been previously reset to 0, then adjusted by Specials, Building effects, etc.
    m_pop.AdjustMax(MaxPopModFromObject(m_object));
    m_health.AdjustMax(MaxHealthModFromObject(m_object));
    if (m_available_food < m_pop.Current()) { // starvation
        // TODO: add starvation special
        // TODO: reduce happiness when it is implemented
        m_health.AdjustMax(PlanetDataTables()["NutrientHealthMod"][0][0]);
    } else if (m_available_food < 2 * m_pop.Current()) {
        // TODO: remove starvation special
        m_health.AdjustMax(PlanetDataTables()["NutrientHealthMod"][0][1]);
    } else if (m_available_food < 4 * m_pop.Current()) {
        // TODO: remove starvation special
        m_health.AdjustMax(PlanetDataTables()["NutrientHealthMod"][0][2]);
    } else {
        // TODO: remove starvation special
        m_health.AdjustMax(PlanetDataTables()["NutrientHealthMod"][0][3]);
    }
}

void PopCenter::PopGrowthProductionResearchPhase()
{
    m_health.AdjustCurrent(m_health.Current() * (((m_health.Max() + 1.0) - m_health.Current()) / (m_health.Max() + 1.0)));
    m_pop.AdjustCurrent(FuturePopGrowth());
}



