#include "PopCenter.h"

#include "../util/AppInterface.h"
#include "../util/DataTable.h"
#include "../util/MultiplayerCommon.h"
#include "../util/OptionsDB.h"
#include "Planet.h"
#include "../util/XMLDoc.h"

#include <boost/lexical_cast.hpp>

#include <stdexcept>

using boost::lexical_cast;

namespace {
    DataTableMap& PlanetDataTables()
    {
        static DataTableMap map;
        if (map.empty()) {
            std::string settings_dir = GetOptionsDB().Get<std::string>("settings-dir");
            if (!settings_dir.empty() && settings_dir[settings_dir.size() - 1] != '/')
                settings_dir += '/';
            LoadDataTables(settings_dir + "planet_tables.txt", map);
        }
        return map;
    }

    double MaxPopModFromObject(const UniverseObject* object)
    {
        if (const Planet* planet = universe_object_cast<const Planet*>(object))
            return PlanetDataTables()["PlanetMaxPop"][planet->Size()][planet->Environment()];
        return 0.0;
    }

    double MaxHealthModFromObject(const UniverseObject* object)
    {
        if (const Planet* planet = universe_object_cast<const Planet*>(object))
            return PlanetDataTables()["PlanetEnvHealthMod"][0][planet->Environment()];
        return 0.0;
    }

    bool temp_header_bool = RecordHeaderFile(PopCenterRevision());
    bool temp_source_bool = RecordSourceFile("$Id$");
}

PopCenter::PopCenter(UniverseObject* object, double max_pop_mod, double max_health_mod) : 
    m_object(object)
{
    assert(m_object);
    Reset(max_pop_mod, max_health_mod);
}

PopCenter::PopCenter(int race, UniverseObject* object, double max_pop_mod, double max_health_mod) : 
    m_object(object)
{
    assert(m_object);
    Reset(max_pop_mod, max_health_mod);
    m_race = race;
}
   
PopCenter::PopCenter(const XMLElement& elem, UniverseObject* object) :
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

XMLElement PopCenter::XMLEncode(UniverseObject::Visibility vis) const
{
    // partial encode version.  PopCenter is always fully visible
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
    return std::min(FuturePopGrowthMax(), std::min(AvailableFood(), m_pop.Max()) - m_pop.Current());
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
}

void PopCenter::PopGrowthProductionResearchPhase()
{
    m_pop.AdjustCurrent(FuturePopGrowth());
    if (AvailableFood() < m_pop.Current()) { // starvation
        m_object->AddSpecial("STARVATION_SPECIAL");
        m_health.AdjustMax(PlanetDataTables()["NutrientHealthMod"][0][0]);
    } else if (m_available_food < 2 * m_pop.Current()) { // "minimal" nutrient levels
        m_object->RemoveSpecial("STARVATION_SPECIAL");
        m_health.AdjustMax(PlanetDataTables()["NutrientHealthMod"][0][1]);
    } else if (m_available_food < 4 * m_pop.Current()) { // "normal" nutrient levels
        m_object->RemoveSpecial("STARVATION_SPECIAL");
        m_health.AdjustMax(PlanetDataTables()["NutrientHealthMod"][0][2]);
    } else { // food orgy!
        m_object->RemoveSpecial("STARVATION_SPECIAL");
        m_health.AdjustMax(PlanetDataTables()["NutrientHealthMod"][0][3]);
    }
    m_health.AdjustCurrent(m_health.Current() * (((m_health.Max() + 1.0) - m_health.Current()) / (m_health.Max() + 1.0)));
}

void PopCenter::Reset(double max_pop_mod, double max_health_mod)
{
    m_pop = Meter(0.0, max_pop_mod);
    m_health = Meter(max_health_mod, max_health_mod);
    m_growth = 0.0;
    m_race = -1;
    m_available_food = 0.0;
}

