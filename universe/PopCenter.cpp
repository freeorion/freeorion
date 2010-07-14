#include "PopCenter.h"

#include "../util/AppInterface.h"
#include "../util/MultiplayerCommon.h"
#include "../util/OptionsDB.h"
#include "../util/Directories.h"
#include "Planet.h"

#include <algorithm>
#include <stdexcept>

const double PopCenter::MINIMUM_POP_CENTER_POPULATION = 0.051;  // rounds up to 0.1 when showing only two digits

class Species;
const Species* GetSpecies(const std::string& name);

PopCenter::PopCenter(const std::string& species_name) :
    m_species_name(species_name), m_allocated_food(0.0)
{
    //Logger().debugStream() << "PopCenter::PopCenter(" << species_name << ")";
    // TODO: if race isn't a valid race, throw a fit
}

PopCenter::PopCenter() :
    m_species_name(""), m_allocated_food(0.0)
{
    //Logger().debugStream() << "PopCenter::PopCenter()";
}

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
        this->m_species_name =      copied_object->m_species_name;
        this->m_allocated_food =    copied_object->m_allocated_food;
    }
}

void PopCenter::Init()
{
    //Logger().debugStream() << "PopCenter::Init";
    AddMeter(METER_POPULATION);
    AddMeter(METER_TARGET_POPULATION);
    AddMeter(METER_HEALTH);
    AddMeter(METER_TARGET_HEALTH);
    AddMeter(METER_FOOD_CONSUMPTION);
}

double PopCenter::CurrentMeterValue(MeterType type) const
{
    const Meter* meter = GetMeter(type);
    if (!meter) {
        throw std::invalid_argument("PopCenter::CurrentMeterValue was passed a MeterType that this PopCenter does not have");
    }
    return meter->Current();
}

double PopCenter::PopCenterNextTurnMeterValue(MeterType meter_type) const
{
    const Meter* meter = GetMeter(meter_type);
    if (!meter) {
        throw std::invalid_argument("PopCenter::PopCenterNextTurnMeterValue passed meter type that the PopCenter does not have.");
    }

    if (meter_type == METER_HEALTH) {
        const Meter* target_meter = GetMeter(METER_TARGET_HEALTH);
        return std::min(target_meter->Current(), meter->Current() + 1.0);

    } else if (meter_type == METER_POPULATION) {
        return meter->Current() + NextTurnPopGrowth();

    } else if (meter_type == METER_FOOD_CONSUMPTION) {
        // guesstimate food consumption next turn, and for full population
        // growth, as a proportional increase in food consumption to the
        // fractional increase in population that is possible.  this could
        // be an inaccurate estimate of next turn's food consumption, but
        // it's the best estimate that can be made
        double next_turn_max_pop_growth = this->NextTurnPopGrowthMax();
        Logger().debugStream() << "PP: ntmpg: " << next_turn_max_pop_growth;
        double current_pop = std::max(0.1, this->CurrentMeterValue(METER_POPULATION));  // floor to effective current population, to prevent overflow issues
        Logger().debugStream() << "PP: curpop: " << current_pop;
        double fractional_growth = next_turn_max_pop_growth / current_pop;
        Logger().debugStream() << "PP: fracgro: " << fractional_growth;
        double expected_next_turn_food_consumption = this->CurrentMeterValue(METER_FOOD_CONSUMPTION) * (1.0 + fractional_growth);
        Logger().debugStream() << "PP: curfdco: " << this->CurrentMeterValue(METER_FOOD_CONSUMPTION);
        Logger().debugStream() << "PP: ntefco: " << expected_next_turn_food_consumption;
        return expected_next_turn_food_consumption;

    } else if (meter_type == METER_TARGET_HEALTH || meter_type == METER_TARGET_POPULATION) {
        Logger().debugStream() << "PopCenter::PopCenterNextTurnMeterValue passed valid but unusual (TARGET) meter_type.  Returning meter->Current()";
        return meter->Current();

    } else {
        Logger().errorStream() << "PopCenter::PopCenterNextTurnMeterValue dealing with invalid meter type";
        return 0.0;
    }
}

double PopCenter::NextTurnPopGrowth() const
{
    double max = GetMeter(METER_TARGET_POPULATION)->Current();
    double cur = GetMeter(METER_POPULATION)->Current();
    //Logger().debugStream() << "PopCenter::NextTurnPopGrowth  growth max: " << NextTurnPopGrowthMax() << "  allocated food: " << AllocatedFood();

    // growth limited by max possible growth from health and current population
    // based formula, by allocated food, and to not increase above the target
    // population
    // decline limited by current population: can't have negative population.

    double raw_growth = std::max(-cur, std::min(NextTurnPopGrowthMax(), std::min(AllocatedFood(), max) - cur));

    // if population will fall below minimum threshold, ensure this function
    // returns actual decrease (although difference would be less than
    // MINIMUM_POP_CENTER_POPULATION in any case
    if (cur - raw_growth < MINIMUM_POP_CENTER_POPULATION)
        return -cur;

    return raw_growth;
}

double PopCenter::NextTurnPopGrowthMax() const
{
    double target_pop = std::max(GetMeter(METER_TARGET_POPULATION)->Current(), 1.0);    // clamping target pop to at least 1 prevents divide by zero cases
    double cur_pop = GetMeter(METER_POPULATION)->Current();
    double cur_health = std::max(GetMeter(METER_HEALTH)->Current(), 120.0);             // clamping current health to at most 120 prevents weird results with overpopulation decay health-dependence

    // There are several factors that combine to produce the actual growth
    // or loss of population on a planet.
    // 1) Growth: If health > 20.0 and population < target pop, population grows
    // 2) Low-Health Decay: If health < 20.0, population decays
    // 3) High-Pop Decay: If population > target pop, population decays

    // if above 20 health and below target population, population can grow
    if (cur_health > 20.0 && cur_pop < cur_health) {
        double underpopulation_fraction = ((target_pop + 1.0) - cur_pop) / target_pop;
        //std::cout << "underpop frac: " << underpopulation_fraction << std::endl;
        double growth_potential = cur_pop * underpopulation_fraction * (cur_health - 20.0) * 0.0005;
        //std::cout << "grow pot: " << growth_potential << std::endl;
        double max_growth = target_pop - cur_pop;       // most pop can grow is up to target pop
        return std::min(max_growth, growth_potential);
    }


    // accumulator for overpopulation and low-health-related loss of population
    double pop_loss_potential = 0.0;

    // if population is above target pop, population decays at a health-dependent rate
    if (cur_pop > target_pop) {
        double overpopulation_fraction = (cur_pop - (target_pop - 1.0)) / target_pop;
        double health_factor = (120.0 - cur_health) / 20.0; // higher health slows overpopulation-related decay, up to 120 health, which stops it
        pop_loss_potential -= overpopulation_fraction * health_factor;
    }

    // if health is less than 20.0, population decays at a health-dependent rate
    if (cur_health < 20.0) {
        // once squared, the fraction of population lost each turn.  About 1%
        // at 18 health, 25% at 10 health, 49% at 6 health, and 90% at 1
        // health.  this gives a slight loss just below 20 health (the break-
        // even point) but and doesn't get critically bad until health drops
        // below 15 or so for extended periods, or below 12 for shorter
        // periods.  In any case, the loss is exponential decay, with a rate
        // depending on the distance below full health.
        double root_fraction = ((20.0 - cur_health) / 20.0);
        pop_loss_potential -= cur_pop*root_fraction*root_fraction;
    }

    double max_loss = -cur_pop; // most pop can decay is to 0 population

    return std::max(max_loss, pop_loss_potential);
}

double PopCenter::NextTurnHealthGrowth() const
{
    double health = CurrentMeterValue(METER_HEALTH);
    double target_health = CurrentMeterValue(METER_TARGET_HEALTH);

    // health rises or falls 1.0 towards target health
    if (health < target_health)
        return std::min(health + 1.0, target_health) - health;
    else
        return std::max(health - 1.0, target_health) - health;
}

void PopCenter::PopCenterResetTargetMaxUnpairedMeters(MeterType meter_type)
{
    if (meter_type == INVALID_METER_TYPE || meter_type == METER_POPULATION)
        GetMeter(METER_TARGET_POPULATION)->ResetCurrent();

    if (meter_type == INVALID_METER_TYPE || meter_type == METER_HEALTH)
        GetMeter(METER_TARGET_HEALTH)->ResetCurrent();

    if (meter_type == INVALID_METER_TYPE || meter_type == METER_FOOD_CONSUMPTION)
        GetMeter(METER_FOOD_CONSUMPTION)->ResetCurrent();
}

void PopCenter::PopCenterPopGrowthProductionResearchPhase()
{
    double cur_pop = CurrentMeterValue(METER_POPULATION);
    double pop_growth = NextTurnPopGrowth();                        // may be negative
    double new_pop = cur_pop + pop_growth;

    GetMeter(METER_HEALTH)->AddToCurrent(NextTurnHealthGrowth());   // change may be negative

    if (new_pop >= MINIMUM_POP_CENTER_POPULATION) {
        GetMeter(METER_POPULATION)->AddToCurrent(pop_growth);
    } else {
        // if population falls below threshold, kill off the remainder
        Reset();
    }
}

void PopCenter::PopCenterClampMeters()
{
    GetMeter(METER_POPULATION)->ClampCurrentToRange();
    GetMeter(METER_TARGET_POPULATION)->ClampCurrentToRange();
    GetMeter(METER_HEALTH)->ClampCurrentToRange();
    GetMeter(METER_TARGET_HEALTH)->ClampCurrentToRange();
    GetMeter(METER_FOOD_CONSUMPTION)->ClampCurrentToRange();
}

void PopCenter::Reset()
{
    GetMeter(METER_POPULATION)->Reset();
    GetMeter(METER_TARGET_POPULATION)->Reset();
    GetMeter(METER_HEALTH)->Reset();
    GetMeter(METER_TARGET_HEALTH)->Reset();
    GetMeter(METER_FOOD_CONSUMPTION)->Reset();
    m_species_name.clear();
    m_allocated_food = 0.0;
}

void PopCenter::SetSpecies(const std::string& species_name)
{
    const Species* species = GetSpecies(species_name);
    if (!species) {
        Logger().errorStream() << "PopCenter::SetSpecies couldn't get species with name " << species_name;
    }
    m_species_name = species_name;
}

void PopCenter::SetAllocatedFood(double allocated_food)
{
    m_allocated_food = allocated_food;
}

