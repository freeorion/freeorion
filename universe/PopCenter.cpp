#include "PopCenter.h"

#include "../util/OptionsDB.h"
#include "../util/Directories.h"
#include "../util/Logger.h"
#include "Meter.h"
#include "Enums.h"

#include <algorithm>
#include <stdexcept>

namespace {
    const double MINIMUM_POP_CENTER_POPULATION = 0.01001;  // rounds up to 0.1 when showing 2 digits, down to 0.05 or 50.0 when showing 3
}

class Species;
const Species* GetSpecies(const std::string& name);

PopCenter::PopCenter(const std::string& species_name) :
    m_species_name(species_name)
{}

PopCenter::PopCenter()
{}

PopCenter::~PopCenter()
{}

void PopCenter::Copy(std::shared_ptr<const PopCenter> copied_object, Visibility vis) {
    if (copied_object.get() == this)
        return;
    if (!copied_object) {
        ErrorLogger() << "PopCenter::Copy passed a null object";
        return;
    }

    if (vis >= VIS_PARTIAL_VISIBILITY) {
        this->m_species_name =      copied_object->m_species_name;
    }
}

void PopCenter::Copy(std::shared_ptr<const PopCenter> copied_object)
{ Copy(copied_object, VIS_FULL_VISIBILITY); }

void PopCenter::Init() {
    AddMeter(METER_POPULATION);
    AddMeter(METER_TARGET_POPULATION);
    AddMeter(METER_HAPPINESS);
    AddMeter(METER_TARGET_HAPPINESS);
}

std::string PopCenter::Dump(unsigned short ntabs) const {
    std::stringstream os;
    os << " species: " << m_species_name << "  ";
    return os.str();
}

float PopCenter::CurrentMeterValue(MeterType type) const {
    const Meter* meter = GetMeter(type);
    if (!meter) {
        throw std::invalid_argument("PopCenter::CurrentMeterValue was passed a MeterType that this PopCenter does not have: " + boost::lexical_cast<std::string>(type));
    }
    return meter->Current();
}

float PopCenter::PopCenterNextTurnMeterValue(MeterType meter_type) const {
    const Meter* meter = GetMeter(meter_type);
    if (!meter) {
        throw std::invalid_argument("PopCenter::PopCenterNextTurnMeterValue passed meter type that the PopCenter does not have: " + boost::lexical_cast<std::string>(meter_type));

    } else if (meter_type == METER_POPULATION) {
        return meter->Current() + NextTurnPopGrowth();

    } else if (meter_type == METER_TARGET_POPULATION ||
               meter_type == METER_TARGET_HAPPINESS)
    {
        DebugLogger() << "PopCenter::PopCenterNextTurnMeterValue passed valid but unusual (TARGET) meter_type" << boost::lexical_cast<std::string>(meter_type) << ".  Returning meter->Current()";
        return meter->Current();

    } else if (meter_type == METER_HAPPINESS) {
        const Meter* target = GetMeter(METER_TARGET_HAPPINESS);
        if (!target)
            return meter->Current();
        float target_meter_value = target->Current();
        float current_meter_value = meter->Current();

        // currently meter growth is one per turn.
        if (target_meter_value > current_meter_value)
            return std::min(current_meter_value + 1.0f, target_meter_value);
        else if (target_meter_value < current_meter_value)
            return std::max(target_meter_value, current_meter_value - 1.0f);
        else
            return current_meter_value;
    } else {
        ErrorLogger() << "PopCenter::PopCenterNextTurnMeterValue dealing with invalid meter type: " + boost::lexical_cast<std::string>(meter_type);
        return 0.0f;
    }
}

float PopCenter::NextTurnPopGrowth() const {
    float target_pop = GetMeter(METER_TARGET_POPULATION)->Current();
    float cur_pop = GetMeter(METER_POPULATION)->Current();
    float pop_change = 0.0f;

    if (target_pop > cur_pop) {
        pop_change = cur_pop * (target_pop + 1 - cur_pop) / 100; // Using target population slightly above actual population avoids excessively slow asymptotic growth towards target.
        pop_change = std::min(pop_change, target_pop - cur_pop);
    } else {
        pop_change = -(cur_pop - target_pop) / 10;
        pop_change = std::max(pop_change, target_pop - cur_pop);
    }

    return pop_change;
}

void PopCenter::PopCenterResetTargetMaxUnpairedMeters() {
    GetMeter(METER_TARGET_POPULATION)->ResetCurrent();
    GetMeter(METER_TARGET_HAPPINESS)->ResetCurrent();
}

void PopCenter::PopCenterPopGrowthProductionResearchPhase() {
    if (m_species_name.empty()) {
        // No changes to population or happiness
        return;
    }

    float cur_pop = CurrentMeterValue(METER_POPULATION);
    float pop_growth = NextTurnPopGrowth(); // may be negative
    float new_pop = cur_pop + pop_growth;

    //if (cur_pop > 0.0)
    //    DebugLogger() << "Planet Pop: " << cur_pop << " growth: " << pop_growth;

    if (new_pop >= MINIMUM_POP_CENTER_POPULATION) {
        GetMeter(METER_POPULATION)->SetCurrent(new_pop);
    } else {
        // if population falls below threshold, kill off the remainder
        Depopulate();
    }

    GetMeter(METER_HAPPINESS)->SetCurrent(PopCenterNextTurnMeterValue(METER_HAPPINESS));
}

void PopCenter::PopCenterClampMeters()
{ GetMeter(METER_POPULATION)->ClampCurrentToRange(); }

void PopCenter::Reset() {
    GetMeter(METER_POPULATION)->Reset();
    GetMeter(METER_TARGET_POPULATION)->Reset();
    GetMeter(METER_HAPPINESS)->Reset();
    GetMeter(METER_TARGET_HAPPINESS)->Reset();
    m_species_name.clear();
}

void PopCenter::Depopulate() {
    GetMeter(METER_POPULATION)->Reset();
    GetMeter(METER_HAPPINESS)->Reset();
}

void PopCenter::SetSpecies(const std::string& species_name) {
    const Species* species = GetSpecies(species_name);
    if (!species && !species_name.empty()) {
        ErrorLogger() << "PopCenter::SetSpecies couldn't get species with name " << species_name;
    }
    m_species_name = species_name;
}
