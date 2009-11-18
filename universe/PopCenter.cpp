#include "PopCenter.h"

#include "../util/AppInterface.h"
#include "../util/DataTable.h"
#include "../util/MultiplayerCommon.h"
#include "../util/OptionsDB.h"
#include "../util/Directories.h"
#include "Planet.h"

#include <algorithm>

#include <boost/lexical_cast.hpp>

#include <stdexcept>

using boost::lexical_cast;

namespace {
    DataTableMap& PlanetDataTables() {
        static DataTableMap map;
        if (map.empty())
            LoadDataTables((GetResourceDir() / "planet_tables.txt").file_string(), map);
        return map;
    }

    double MaxPopModFromObject(const UniverseObject* object) {
        if (const Planet* planet = universe_object_cast<const Planet*>(object))
            return PlanetDataTables()["PlanetMaxPop"][planet->Size()][planet->Environment()];
        return 0.0;
    }

    double MaxHealthModFromObject(const UniverseObject* object) {
        if (const Planet* planet = universe_object_cast<const Planet*>(object))
            return PlanetDataTables()["PlanetEnvHealthMod"][0][planet->Environment()];
        return 0.0;
    }
}

const double PopCenter::MINIMUM_POP_CENTER_POPULATION = 0.051;  // rounds up to 0.1 when showing only two digits

PopCenter::PopCenter(int race) :
    m_race(race), m_allocated_food(0.0)
{}

PopCenter::PopCenter() :
    m_race(-1), m_allocated_food(0.0)
{}

PopCenter::~PopCenter()
{}

void PopCenter::Copy(const PopCenter* copied_object, Visibility vis)
{
    if (copied_object == this)
        return;
    if (!copied_object) {
        Logger().errorStream() << "PopCenter::Copy passed a null object";
        return;
    }

    if (vis >= VIS_PARTIAL_VISIBILITY) {
        this->m_race =              copied_object->m_race;
        this->m_allocated_food =    copied_object->m_allocated_food;
    }
}

void PopCenter::Init(double max_pop_mod, double max_health_mod)
{
    InsertMeter(METER_POPULATION, Meter());
    InsertMeter(METER_HEALTH, Meter());
    Reset(max_pop_mod, max_health_mod);
}

double PopCenter::Inhabitants() const
{
    return GetMeter(METER_POPULATION)->Current();    // TO DO: Something fancy for different races
}

double PopCenter::FuturePopGrowth() const
{
    double max = GetMeter(METER_POPULATION)->Max();
    double cur = GetMeter(METER_POPULATION)->Current();
    //Logger().debugStream() << "PopCenter::FuturePopGrowth  growth max: " << FuturePopGrowthMax() << "  allocated food: " << AllocatedFood();

    // growth limited by max possible growth from health and current population
    // based formula, by allocated food, and by max possible population for
    // this PopCenter.
    // population decline limited by current population: can't have negative
    // population.
    double raw_growth = std::max(-cur, std::min(FuturePopGrowthMax(), std::min(AllocatedFood(), max) - cur));

    // if population will fall below minimum threshold, ensure this function
    // returns actual decrease (although difference would be less than
    // MINIMUM_POP_CENTER_POPULATION in any case
    if (cur - raw_growth < MINIMUM_POP_CENTER_POPULATION)
        return -cur;

    return raw_growth;
}

double PopCenter::FuturePopGrowthMax() const
{
    double max_pop = GetMeter(METER_POPULATION)->Max();
    double cur_pop = GetMeter(METER_POPULATION)->Current();
    double cur_health = GetMeter(METER_HEALTH)->Current();

    if (20.0 < cur_health) {
        return std::min(max_pop - cur_pop, cur_pop * (((max_pop + 1.0) - cur_pop) / (max_pop + 1.0)) * (cur_health - 20.0) * 0.01);
    } else if (cur_health == 20.0) {
        return 0.0;
    } else { // cur_health < 20.0
        // once squared, the fraction of population lost each turn.  About 1%
        // at 18 health, 25% at 10 health, 49% at 6 health, and 90% at 1
        // health.  this gives a slight loss just below 20 health (the break-
        // even point) but and doesn't get critically bad until health drops
        // below 15 or so for extended periods, or below 12 for shorter
        // periods.  In any case, the loss is exponential decay, with a rate
        // depending on the distance below full health.
        double root_fraction = ((20.0 - cur_health) / 20.0);
        return std::max(-cur_pop, -cur_pop*(root_fraction*root_fraction));
    }
}

double PopCenter::FutureHealthGrowth() const
{
    double max = GetMeter(METER_HEALTH)->Max();
    double cur = GetMeter(METER_HEALTH)->Current();
    return std::min(max - cur, 1.0);
}

double PopCenter::ProjectedCurrentMeter(MeterType type) const
{
    switch (type) {
    case METER_POPULATION:
        return GetMeter(METER_POPULATION)->InitialCurrent() + FuturePopGrowth();
        break;
    case METER_HEALTH:
        return GetMeter(METER_HEALTH)->InitialCurrent() + FutureHealthGrowth();
        break;
    default:
        const UniverseObject* obj = dynamic_cast<const UniverseObject*>(this);
        if (obj)
            return obj->ProjectedCurrentMeter(type);
        else
            throw std::runtime_error("PopCenter::ProjectedCurrentMeter couldn't convert this pointer to UniverseObject*");
    }
}

double PopCenter::MeterPoints(MeterType type) const
{
    switch (type) {
    case METER_POPULATION:
    case METER_HEALTH:
        return GetMeter(type)->InitialCurrent();    // health and population point values are equal to meter values
        break;
    default:
        const UniverseObject* obj = dynamic_cast<const UniverseObject*>(this);
        if (obj)
            return obj->MeterPoints(type);
        else
            throw std::runtime_error("PopCenter::MeterPoints couldn't convert this pointer to UniverseObject*");
    }
}

double PopCenter::ProjectedMeterPoints(MeterType type) const
{
    switch (type) {
    case METER_POPULATION:
    case METER_HEALTH:
        return ProjectedCurrentMeter(type);
        break;
    default:
        const UniverseObject* obj = dynamic_cast<const UniverseObject*>(this);
        if (obj)
            return obj->ProjectedMeterPoints(type);
        else
            throw std::runtime_error("PopCenter::ProjectedMeterPoints couldn't convert this pointer to UniverseObject*");
    }
}

void PopCenter::ApplyUniverseTableMaxMeterAdjustments(MeterType meter_type)
{
    const UniverseObject* object = GetThisObject();
    if (!object) {
        Logger().errorStream() << "PopCenter GetThisObject returned 0";
        return;
    }

    if (meter_type == INVALID_METER_TYPE || meter_type == METER_POPULATION)
        GetMeter(METER_POPULATION)->AdjustMax(MaxPopModFromObject(object));

    if (meter_type == INVALID_METER_TYPE || meter_type == METER_HEALTH)
        GetMeter(METER_HEALTH)->AdjustMax(MaxHealthModFromObject(object));
}

void PopCenter::PopGrowthProductionResearchPhase()
{
    Meter* pop = GetMeter(METER_POPULATION);
    Meter* health = GetMeter(METER_HEALTH);

    double pop_growth = FuturePopGrowth();
    double cur = pop->Current();
    double new_pop = cur + pop_growth;
    if (new_pop >= MINIMUM_POP_CENTER_POPULATION) {
        pop->SetCurrent(new_pop);

        double health_delta = FutureHealthGrowth();
        health->AdjustCurrent(health_delta);
    } else {
        // if population falls below threshold, fall completely to zero.
        pop->SetCurrent(0.0);
        health->SetCurrent(0.0);
    }
}

void PopCenter::Reset(double max_pop_mod, double max_health_mod)
{
    GetMeter(METER_POPULATION)->Set(0.0, max_pop_mod);
    GetMeter(METER_HEALTH)->Set(max_health_mod, max_health_mod);
    m_race = -1;
    m_allocated_food = 0.0;
}
