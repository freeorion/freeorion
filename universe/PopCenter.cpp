#include "PopCenter.h"

#include "../util/AppInterface.h"
#include "../util/MultiplayerCommon.h"
#include "../util/OptionsDB.h"
#include "../util/Directories.h"
#include "Planet.h"

#include <algorithm>
#include <stdexcept>

namespace {
    const double MINIMUM_POP_CENTER_POPULATION = 0.051;  // rounds up to 0.1 when showing only two digits
}

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

std::string PopCenter::Dump() const
{
    std::stringstream os;
    os << " species: " << m_species_name << " allocated food: " << m_allocated_food;
    return os.str();
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
        return meter->Current() + NextTurnHealthGrowth();

    } else if (meter_type == METER_POPULATION) {
        return meter->Current() + NextTurnPopGrowth();

    } else if (meter_type == METER_FOOD_CONSUMPTION) {
        // Guesstimate food consumption next turn.
        // Assume an equal fractional increase in food consumption as the
        // fractional increase in population that is expected (which depends on
        // the current food allocation).
        // this will be an accurate estimate of next turn's food consumption
        // unless food consumption is not proportional to population
        double next_turn_pop_growth = this->NextTurnPopGrowth();
        double current_pop = std::max(MINIMUM_POP_CENTER_POPULATION, this->CurrentMeterValue(METER_POPULATION));  // floor to effective current population, to prevent overflow issues
        double fractional_growth = next_turn_pop_growth / current_pop;
        double expected_next_turn_food_consumption = this->CurrentMeterValue(METER_FOOD_CONSUMPTION) * (1.0 + fractional_growth);

        //Logger().debugStream() << "PopCenter::PopCenterNextTurnMeterValue(FOOD CONSUMPTION)" <<
        //    " next turn growth max: " << next_turn_pop_growth <<
        //    " current pop: " << current_pop <<
        //    " fractional growth: " << fractional_growth <<
        //    " current food consumption: " << this->CurrentMeterValue(METER_FOOD_CONSUMPTION) <<
        //    " expected next turn food consumption: " << expected_next_turn_food_consumption;

        return expected_next_turn_food_consumption;

    } else if (meter_type == METER_TARGET_HEALTH || meter_type == METER_TARGET_POPULATION) {
        Logger().debugStream() << "PopCenter::PopCenterNextTurnMeterValue passed valid but unusual (TARGET) meter_type.  Returning meter->Current()";
        return meter->Current();

    } else {
        Logger().errorStream() << "PopCenter::PopCenterNextTurnMeterValue dealing with invalid meter type";
        return 0.0;
    }
}

double PopCenter::FoodAllocationForMaxGrowth() const
{
    // the same as PopCenterNextTurnMeterValue(METER_FOOD_ALLOCATION), but
    // using NextTurnPopGrowhtMax instead of NextTurnPopGrowth
    double next_turn_max_pop_growth = this->NextTurnPopGrowthMax();
    double current_pop = std::max(MINIMUM_POP_CENTER_POPULATION, this->CurrentMeterValue(METER_POPULATION));  // floor to effective current population, to prevent overflow issues
    double fractional_max_growth = next_turn_max_pop_growth / current_pop;
    double max_growth_next_turn_food_consumption = this->CurrentMeterValue(METER_FOOD_CONSUMPTION) * (1.0 + fractional_max_growth);
    return max_growth_next_turn_food_consumption;
}

double PopCenter::NextTurnPopGrowth() const
{
    //Logger().debugStream() << "PopCenter::NextTurnPopGrowth  growth max: " << NextTurnPopGrowthMax() << "  allocated food: " << AllocatedFood();

    double next_turn_pop_growth_max = NextTurnPopGrowthMax();       // before food considerations
    double current_pop = std::max(MINIMUM_POP_CENTER_POPULATION, this->CurrentMeterValue(METER_POPULATION));    // floor to effective current population, to prevent overflow issues
    double need_for_stable_population = this->CurrentMeterValue(METER_FOOD_CONSUMPTION);

    // if food need for stable population is sufficiently small, ignore any
    // food allocation requirements when determining growth.  This lets races
    // that don't require food grow without need to be allocated any, and
    // avoids any divide by zero issues.
    //
    // this may however cause issues when at small populations that need
    // less than one food per unit of pop, when a population could end up
    // growing for a few turns until it gets big enough to start needing food,
    // at which point it could starve...
    if (need_for_stable_population < MINIMUM_POP_CENTER_POPULATION) {
        double new_pop = current_pop + next_turn_pop_growth_max;
        if (new_pop < MINIMUM_POP_CENTER_POPULATION)
            next_turn_pop_growth_max = -current_pop;
        return next_turn_pop_growth_max;
    }

    // determine fraction of required food that was allocated.
    double allocated_food = AllocatedFood();
    double food_allocation_fraction = allocated_food / need_for_stable_population;

    double population_supportable_by_allocated_food = current_pop * food_allocation_fraction;

    double new_pop = std::min(current_pop + next_turn_pop_growth_max, population_supportable_by_allocated_food);
    if (new_pop < MINIMUM_POP_CENTER_POPULATION)
        new_pop = 0.0;

    //Logger().debugStream() << "PopCenter::NextTurnPopGrowth() allocated food: " << allocated_food <<
    //    " food allocation fraction: " << food_allocation_fraction <<
    //    " supportable pop: " << population_supportable_by_allocated_food <<
    //    " new pop: " << new_pop;

    return new_pop - current_pop;
}

double PopCenter::NextTurnPopGrowthMax() const
{
    double target_pop = std::max(GetMeter(METER_TARGET_POPULATION)->Current(), 1.0);    // clamping target pop to at least 1 prevents divide by zero cases
    double cur_pop = GetMeter(METER_POPULATION)->Current();
    double cur_health = std::min(GetMeter(METER_HEALTH)->Current(), 120.0);             // clamping current health to at most 120 prevents weird results with overpopulation decay health-dependence

    // There are several factors that combine to produce the actual growth
    // or loss of population on a planet.
    // 1) Growth: If health > 20.0 and population < target pop, population grows
    // 2) Low-Health Decay: If health < 20.0, population decays
    // 3) High-Pop Decay: If population > target pop, population decays

    // if above 20 health and below target population, population can grow
    if (cur_health > 20.0 && cur_pop < target_pop) {
        double underpopulation_fraction = ((target_pop + 1.0) - cur_pop) / target_pop;
        Logger().debugStream() << "underpop frac: " << underpopulation_fraction;
        double growth_potential = cur_pop * underpopulation_fraction * (cur_health - 20.0) * 0.005;
        Logger().debugStream() << "grow pot: " << growth_potential;
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

    Logger().debugStream() << "Planet Pop: " << cur_pop << " growth: " << pop_growth;

    GetMeter(METER_HEALTH)->AddToCurrent(NextTurnHealthGrowth());   // change may be negative

    if (new_pop >= MINIMUM_POP_CENTER_POPULATION) {
        GetMeter(METER_POPULATION)->AddToCurrent(pop_growth);
        double new_population = CurrentMeterValue(METER_POPULATION);

        double new_pop_fraction_of_old_pop = new_population / cur_pop;  // cur_pop is now old pop value

        // update food allocation after growth; otherwise current value will be that for pre-growth population
        if (Meter* food_consumption_meter = GetMeter(METER_FOOD_CONSUMPTION)) {
            double estimated_new_food_allocation_after_growth = food_consumption_meter->Current() * new_pop_fraction_of_old_pop;
            food_consumption_meter->SetCurrent(estimated_new_food_allocation_after_growth);
        }

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

