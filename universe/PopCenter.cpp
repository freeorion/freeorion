#include "PopCenter.h"

#include <algorithm>
#include <stdexcept>
#include "Enums.h"
#include "Meter.h"
#include "Species.h"
#include "UniverseObject.h"
#include "../util/AppInterface.h"
#include "../util/Logger.h"
#include "../util/OptionsDB.h"


namespace {
    constexpr double MINIMUM_POP_CENTER_POPULATION = 0.01001;  // rounds up to 0.1 when showing 2 digits, down to 0.05 or 50.0 when showing 3
}

PopCenter::PopCenter(const std::string& species_name) :
    m_species_name(species_name)
{}

void PopCenter::Copy(const PopCenter& copied_object, Visibility vis) {
    if (&copied_object == this)
        return;

    if (vis >= Visibility::VIS_PARTIAL_VISIBILITY)
        this->m_species_name = copied_object.m_species_name;
}

void PopCenter::Copy(const PopCenter& copied_object)
{ Copy(copied_object, Visibility::VIS_FULL_VISIBILITY); }

void PopCenter::Init() {
    AddMeter(MeterType::METER_POPULATION);
    AddMeter(MeterType::METER_TARGET_POPULATION);
    AddMeter(MeterType::METER_HAPPINESS);
    AddMeter(MeterType::METER_TARGET_HAPPINESS);
}

std::string PopCenter::Dump(uint8_t ntabs) const
{ return std::string{" species: "}.append(m_species_name).append("  "); }

bool PopCenter::Populated() const
{ return GetMeter(MeterType::METER_POPULATION)->Current() >= MINIMUM_POP_CENTER_POPULATION; }

void PopCenter::PopCenterResetTargetMaxUnpairedMeters() {
    GetMeter(MeterType::METER_TARGET_POPULATION)->ResetCurrent();
    GetMeter(MeterType::METER_TARGET_HAPPINESS)->ResetCurrent();
}

void PopCenter::PopCenterPopGrowthProductionResearchPhase(int turn) {
    if (m_species_name.empty()) {
        // No changes to population or happiness
        return;
    }

    // Should be run after meter update but before a backpropagation, so check current, not initial, meter values

    if (!Populated()) {
        // if population falls below threshold, kill off the remainder
        Depopulate(turn);
    }
}

void PopCenter::PopCenterClampMeters()
{ GetMeter(MeterType::METER_POPULATION)->ClampCurrentToRange(); }

void PopCenter::Reset(ObjectMap&) {
    GetMeter(MeterType::METER_POPULATION)->Reset();
    GetMeter(MeterType::METER_TARGET_POPULATION)->Reset();
    GetMeter(MeterType::METER_HAPPINESS)->Reset();
    GetMeter(MeterType::METER_TARGET_HAPPINESS)->Reset();
    m_species_name.clear();
}

void PopCenter::Depopulate(int) {
    GetMeter(MeterType::METER_POPULATION)->Reset();
    GetMeter(MeterType::METER_HAPPINESS)->Reset();
}

void PopCenter::SetSpecies(std::string species_name, int, const SpeciesManager& sm) {
    if (!species_name.empty() && !sm.GetSpecies(species_name))
        ErrorLogger() << "PopCenter::SetSpecies couldn't get species with name " << species_name;
    m_species_name = std::move(species_name);
}
