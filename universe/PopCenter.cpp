#include "PopCenter.h"

#include "../util/AppInterface.h"
#include "../util/MultiplayerCommon.h"
#include "../util/OptionsDB.h"
#include "../util/Directories.h"
#include "Planet.h"

#include <algorithm>
#include <stdexcept>

namespace {
    const double MINIMUM_POP_CENTER_POPULATION = 0.05001;  // rounds up to 0.1 when showing 2 digits, down to 0.05 or 50.0 when showing 3
}

class Species;
const Species* GetSpecies(const std::string& name);

PopCenter::PopCenter(const std::string& species_name) :
    m_species_name(species_name)
{}

PopCenter::PopCenter() :
    m_species_name("")
{}

PopCenter::~PopCenter()
{}

void PopCenter::Copy(const PopCenter* copied_object, Visibility vis) {
    if (copied_object == this)
        return;
    if (!copied_object) {
        Logger().errorStream() << "PopCenter::Copy passed a null object";
        return;
    }

    if (vis >= VIS_PARTIAL_VISIBILITY) {
        this->m_species_name =      copied_object->m_species_name;
    }
}

void PopCenter::Init() {
    //Logger().debugStream() << "PopCenter::Init";
    AddMeter(METER_POPULATION);
    AddMeter(METER_TARGET_POPULATION);
}

std::string PopCenter::Dump() const {
    std::stringstream os;
    os << " species: " << m_species_name << "  ";
    return os.str();
}

double PopCenter::CurrentMeterValue(MeterType type) const {
    const Meter* meter = GetMeter(type);
    if (!meter) {
        throw std::invalid_argument("PopCenter::CurrentMeterValue was passed a MeterType that this PopCenter does not have");
    }
    return meter->Current();
}

double PopCenter::PopCenterNextTurnMeterValue(MeterType meter_type) const {
    const Meter* meter = GetMeter(meter_type);
    if (!meter) {
        throw std::invalid_argument("PopCenter::PopCenterNextTurnMeterValue passed meter type that the PopCenter does not have.");

    } else if (meter_type == METER_POPULATION) {
        return meter->Current() + NextTurnPopGrowth();

    } else if (meter_type == METER_TARGET_POPULATION) {
        Logger().debugStream() << "PopCenter::PopCenterNextTurnMeterValue passed valid but unusual (TARGET) meter_type.  Returning meter->Current()";
        return meter->Current();

    } else {
        Logger().errorStream() << "PopCenter::PopCenterNextTurnMeterValue dealing with invalid meter type";
        return 0.0;
    }
}

double PopCenter::NextTurnPopGrowth() const {
    double target_pop = std::max(GetMeter(METER_TARGET_POPULATION)->Current(), 0.01);   // clamping target pop to at least 1 prevents divide by zero cases
    double cur_pop = GetMeter(METER_POPULATION)->Current();
    Logger().debugStream() << "pop: " << cur_pop << " / " << target_pop;
    double population_fraction = (target_pop - cur_pop) / target_pop;
    Logger().debugStream() << "pop frac: " << population_fraction;
    double change_potential = cur_pop * population_fraction * 0.1 + (target_pop > cur_pop ? 1 : -1) * 0.05;
    Logger().debugStream() << "change potential: " << change_potential;
    double max_growth = target_pop - cur_pop;
    double change = 0.0;
    if (change_potential > 0)
        change = std::min(max_growth, change_potential);
    else if (change_potential < 0)
        change = std::max(max_growth, change_potential);
    Logger().debugStream() << "pop change: " << change;
    return change;
}

void PopCenter::PopCenterResetTargetMaxUnpairedMeters()
{ GetMeter(METER_TARGET_POPULATION)->ResetCurrent(); }

void PopCenter::PopCenterPopGrowthProductionResearchPhase() {
    double cur_pop = CurrentMeterValue(METER_POPULATION);
    double pop_growth = NextTurnPopGrowth();                        // may be negative
    double new_pop = cur_pop + pop_growth;

    Logger().debugStream() << "Planet Pop: " << cur_pop << " growth: " << pop_growth;

    if (new_pop >= MINIMUM_POP_CENTER_POPULATION) {
        GetMeter(METER_POPULATION)->SetCurrent(new_pop);
    } else {
        // if population falls below threshold, kill off the remainder
        Reset();
    }
}

void PopCenter::PopCenterClampMeters() {
    GetMeter(METER_TARGET_POPULATION)->ClampCurrentToRange();
    GetMeter(METER_POPULATION)->ClampCurrentToRange();
}

void PopCenter::Reset() {
    GetMeter(METER_POPULATION)->Reset();
    GetMeter(METER_TARGET_POPULATION)->Reset();
    m_species_name.clear();
}

void PopCenter::SetSpecies(const std::string& species_name) {
    const Species* species = GetSpecies(species_name);
    if (!species && !species_name.empty()) {
        Logger().errorStream() << "PopCenter::SetSpecies couldn't get species with name " << species_name;
    }
    m_species_name = species_name;
}
