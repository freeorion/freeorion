#include "PopCenter.h"

#include <algorithm>
#include <stdexcept>
#include "Enums.h"
#include "Meter.h"
#include "UniverseObject.h"
#include "../util/AppInterface.h"
#include "../util/Logger.h"
#include "../util/OptionsDB.h"


namespace {
    const double MINIMUM_POP_CENTER_POPULATION = 0.01001;  // rounds up to 0.1 when showing 2 digits, down to 0.05 or 50.0 when showing 3
}

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

    if (vis >= Visibility::VIS_PARTIAL_VISIBILITY) {
        this->m_species_name =      copied_object->m_species_name;
    }
}

void PopCenter::Copy(std::shared_ptr<const PopCenter> copied_object)
{ Copy(copied_object, Visibility::VIS_FULL_VISIBILITY); }

void PopCenter::Init() {
    AddMeter(MeterType::METER_POPULATION);
    AddMeter(MeterType::METER_TARGET_POPULATION);
    AddMeter(MeterType::METER_HAPPINESS);
    AddMeter(MeterType::METER_TARGET_HAPPINESS);
}

std::string PopCenter::Dump(unsigned short ntabs) const {
    std::stringstream os;
    os << " species: " << m_species_name << "  ";
    return os.str();
}

bool PopCenter::Populated() const
{ return GetMeter(MeterType::METER_POPULATION)->Current() >= MINIMUM_POP_CENTER_POPULATION; }

void PopCenter::PopCenterResetTargetMaxUnpairedMeters() {
    GetMeter(MeterType::METER_TARGET_POPULATION)->ResetCurrent();
    GetMeter(MeterType::METER_TARGET_HAPPINESS)->ResetCurrent();
}

void PopCenter::PopCenterPopGrowthProductionResearchPhase() {
    if (m_species_name.empty()) {
        // No changes to population or happiness
        return;
    }

    // Should be run after meter update but before a backpropagation, so check current, not initial, meter values

    if (!Populated()) {
        // if population falls below threshold, kill off the remainder
        Depopulate();
    }
}

void PopCenter::PopCenterClampMeters()
{ GetMeter(MeterType::METER_POPULATION)->ClampCurrentToRange(); }

void PopCenter::Reset() {
    GetMeter(MeterType::METER_POPULATION)->Reset();
    GetMeter(MeterType::METER_TARGET_POPULATION)->Reset();
    GetMeter(MeterType::METER_HAPPINESS)->Reset();
    GetMeter(MeterType::METER_TARGET_HAPPINESS)->Reset();
    m_species_name.clear();
}

void PopCenter::Depopulate() {
    GetMeter(MeterType::METER_POPULATION)->Reset();
    GetMeter(MeterType::METER_HAPPINESS)->Reset();
}

void PopCenter::SetSpecies(std::string species_name) {
    if (!species_name.empty() && !GetSpecies(species_name))
        ErrorLogger() << "PopCenter::SetSpecies couldn't get species with name " << species_name;
    m_species_name = std::move(species_name);
}
