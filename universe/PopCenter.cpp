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

void PopCenter::PopCenterResetTargetMaxUnpairedMeters() {
    GetMeter(METER_TARGET_POPULATION)->ResetCurrent();
    GetMeter(METER_TARGET_HAPPINESS)->ResetCurrent();
}

void PopCenter::PopCenterPopGrowthProductionResearchPhase() {
    if (m_species_name.empty()) {
        // No changes to population or happiness
        return;
    }

    // Should be run after meter update but before a backpropagation, so check current, not initial, meter values

    if (CurrentMeterValue(METER_POPULATION) < MINIMUM_POP_CENTER_POPULATION) {
        // if population falls below threshold, kill off the remainder
        Depopulate();
    }
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
    if (!species_name.empty() && !GetSpecies(species_name)) {
        ErrorLogger() << "PopCenter::SetSpecies couldn't get species with name "
                      << species_name;
    }
    m_species_name = species_name;
}
