#include "PopCenter.h"

#include "../util/AppInterface.h"
#include "../util/DataTable.h"
#include "../util/MultiplayerCommon.h"
#include "../util/OptionsDB.h"
#include "Planet.h"

#include <algorithm>

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

}

PopCenter::PopCenter() :
    m_growth(0),
    m_race(0),
    m_available_food(0),
    m_pop(),
    m_health()
{}

PopCenter::PopCenter(double max_pop_mod, double max_health_mod) :
    m_growth(0),
    m_race(0),
    m_available_food(0),
    m_pop(),
    m_health()
{
    Reset(max_pop_mod, max_health_mod);
}

PopCenter::PopCenter(int race, double max_pop_mod, double max_health_mod) :
    m_growth(0),
    m_race(race),
    m_available_food(0),
    m_pop(),
    m_health()
{
    Reset(max_pop_mod, max_health_mod);
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
    Logger().debugStream() << "PopCenter::FuturePopGrowth(): id: " << dynamic_cast<const UniverseObject* const>(this)->ID();
    Logger().debugStream() << "FuturePopGrowthMax(): " << FuturePopGrowthMax();
    Logger().debugStream() << "std::min(AvailableFood(), m_pop.Max()) - m_pop.Current(): " << std::min(AvailableFood(), m_pop.Max()) - m_pop.Current();
    return std::max(-m_pop.Current(), std::min(FuturePopGrowthMax(), std::min(AvailableFood(), m_pop.Max()) - m_pop.Current()));
}

double PopCenter::FuturePopGrowthMax() const
{
    Logger().debugStream() << "PopCenter::FuturePopGrowthMax(): id: " << dynamic_cast<const UniverseObject* const>(this)->ID();
    if (20.0 < m_health.Current()) {
        Logger().debugStream() << "health " << m_health.Current() <<" > 20";
        Logger().debugStream() << "m_pop.Max(): " << m_pop.Max();
        Logger().debugStream() << "m_pop.Current(): " << m_pop.Current();
        return std::min(m_pop.Max() - m_pop.Current(), m_pop.Current() * (((m_pop.Max() + 1.0) - m_pop.Current()) / (m_pop.Max() + 1.0)) * (m_health.Current() - 20.0) * 0.01);
    } else if (m_health.Current() == 20.0) {
        return 0.0;
    } else { // m_health.Current() < 20.0
        Logger().debugStream() << "health " << m_health.Current() << " < 20";
        Logger().debugStream() << "m_pop.Max(): " << m_pop.Max();
        Logger().debugStream() << "m_pop.Current(): " << m_pop.Current();
        return std::max(-m_pop.Current(), -m_pop.Current()*(  exp( (m_health.Current()-20)*(m_health.Current()-20) / (400/log(2.0)) ) - 1  ));
    }
}

double PopCenter::FutureHealthGrowth() const
{
    return std::min(MaxHealth() - Health(), m_health.Current() * (((m_health.Max() + 1.0) - m_health.Current()) / (m_health.Max() + 1.0)));
}

void PopCenter::ApplyUniverseTableMaxMeterAdjustments()
{
    UniverseObject* object = GetObjectSignal();
    assert(object);
    // determine meter maxes; they should have been previously reset to 0, then adjusted by Specials, Building effects, etc.
    m_pop.AdjustMax(MaxPopModFromObject(object));
    m_health.AdjustMax(MaxHealthModFromObject(object));
}

void PopCenter::PopGrowthProductionResearchPhase()
{
    UniverseObject* object = GetObjectSignal();
    assert(object);
    m_pop.AdjustCurrent(FuturePopGrowth());
    if (AvailableFood() < m_pop.Current()) { // starvation
        object->AddSpecial("STARVATION_SPECIAL");
        m_health.AdjustMax(PlanetDataTables()["NutrientHealthMod"][0][0]);
    } else if (m_available_food < 2 * m_pop.Current()) { // "minimal" nutrient levels
        object->RemoveSpecial("STARVATION_SPECIAL");
        m_health.AdjustMax(PlanetDataTables()["NutrientHealthMod"][0][1]);
    } else if (m_available_food < 4 * m_pop.Current()) { // "normal" nutrient levels
        object->RemoveSpecial("STARVATION_SPECIAL");
        m_health.AdjustMax(PlanetDataTables()["NutrientHealthMod"][0][2]);
    } else { // food orgy!
        object->RemoveSpecial("STARVATION_SPECIAL");
        m_health.AdjustMax(PlanetDataTables()["NutrientHealthMod"][0][3]);
    }
    m_health.AdjustCurrent(m_health.Current() * (((m_health.Max() + 1.0) - m_health.Current()) / (m_health.Max() + 1.0)));
}

void PopCenter::Reset(double max_pop_mod, double max_health_mod)
{
    m_pop =     Meter(0.0,              max_pop_mod,    m_pop.InitialCurrent(),     m_pop.InitialMax(),     m_pop.PreviousCurrent(),    m_pop.PreviousMax());
    m_health =  Meter(max_health_mod,   max_health_mod, m_health.InitialCurrent(),  m_health.InitialMax(),  m_health.PreviousCurrent(), m_health.PreviousMax());
    m_growth = 0.0;
    m_race = -1;
    m_available_food = 0.0;
}
