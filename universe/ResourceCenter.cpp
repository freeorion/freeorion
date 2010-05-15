#include "ResourceCenter.h"

#include "../util/AppInterface.h"
#include "../util/Directories.h"
#include "../util/OptionsDB.h"
#include "../util/MultiplayerCommon.h"
#include "../Empire/Empire.h"
#include "Fleet.h"
#include "Planet.h"
#include "ShipDesign.h"
#include "System.h"
#include "Building.h"

#include <boost/lexical_cast.hpp>

#include <stdexcept>

using boost::lexical_cast;

namespace {
    const double PRIMARY_FOCUS_BONUS = 15.0;
    const double PRIMARY_BALANCED_BONUS = 3.0;
    const double SECONDARY_FOCUS_BONUS = 5.0;
    const double SECONDARY_BALANCED_BONUS = 1.0;

    const double METER_GROWTH_RATE = 1.0;
}

ResourceCenter::ResourceCenter() :
    m_primary(INVALID_FOCUS_TYPE),
    m_secondary(INVALID_FOCUS_TYPE)
{
    //Logger().debugStream() << "ResourceCenter::ResourceCenter()";
}

ResourceCenter::~ResourceCenter()
{}

ResourceCenter::ResourceCenter(const ResourceCenter& rhs) :
    m_primary(rhs.m_primary),
    m_secondary(rhs.m_secondary)
{}

void ResourceCenter::Copy(const ResourceCenter* copied_object, Visibility vis)
{
    if (copied_object == this)
        return;
    if (!copied_object) {
        Logger().errorStream() << "ResourceCenter::Copy passed a null object";
        return;
    }

    if (vis == VIS_FULL_VISIBILITY) {
        this->m_primary =   copied_object->m_primary;
        this->m_secondary = copied_object->m_secondary;
    }
}

void ResourceCenter::Init()
{
    //Logger().debugStream() << "ResourceCenter::Init";
    AddMeter(METER_FARMING);
    AddMeter(METER_MINING);
    AddMeter(METER_INDUSTRY);
    AddMeter(METER_RESEARCH);
    AddMeter(METER_TRADE);
    AddMeter(METER_CONSTRUCTION);
    AddMeter(METER_TARGET_FARMING);
    AddMeter(METER_TARGET_MINING);
    AddMeter(METER_TARGET_INDUSTRY);
    AddMeter(METER_TARGET_RESEARCH);
    AddMeter(METER_TARGET_TRADE);
    AddMeter(METER_TARGET_CONSTRUCTION);
    m_primary = INVALID_FOCUS_TYPE;
    m_secondary = INVALID_FOCUS_TYPE;
}

double ResourceCenter::ResourceCenterNextTurnMeterValue(MeterType type) const
{
    const Meter* meter = GetMeter(type);
    if (!meter) {
        throw std::invalid_argument("ResourceCenter::ResourceCenterNextTurnMeterValue passed meter type that the ResourceCenter does not have.");
    }
    double current_meter_value = meter->Current();

    MeterType target_meter_type = INVALID_METER_TYPE;
    switch (type) {
    case METER_TARGET_FARMING:
    case METER_TARGET_MINING:
    case METER_TARGET_INDUSTRY:
    case METER_TARGET_RESEARCH:
    case METER_TARGET_TRADE:
    case METER_TARGET_CONSTRUCTION:
        return current_meter_value;
        break;
    case METER_FARMING:     target_meter_type = METER_TARGET_FARMING;       break;
    case METER_MINING:      target_meter_type = METER_TARGET_MINING;        break;
    case METER_INDUSTRY:    target_meter_type = METER_TARGET_INDUSTRY;      break;
    case METER_RESEARCH:    target_meter_type = METER_TARGET_RESEARCH;      break;
    case METER_TRADE:       target_meter_type = METER_TARGET_TRADE;         break;
    case METER_CONSTRUCTION:target_meter_type = METER_TARGET_CONSTRUCTION;  break;
    default:
        Logger().errorStream() << "ResourceCenter::ResourceCenterNextTurnMeterValue dealing with invalid meter type";
        return 0.0;
    }

    const Meter* target_meter = GetMeter(target_meter_type);
    if (!target_meter) {
        throw std::runtime_error("ResourceCenter::ResourceCenterNextTurnMeterValue dealing with invalid meter type");
    }
    double target_meter_value = target_meter->Current();

    // currently meter growth is one per turn.
    if (target_meter_value > current_meter_value)
        return std::min(current_meter_value + 1.0, target_meter_value);
    else if (target_meter_value < current_meter_value)
         return std::max(target_meter_value, current_meter_value - 1.0);
    else
        return current_meter_value;
}

void ResourceCenter::SetPrimaryFocus(FocusType focus)
{
    m_primary = focus;
    ResourceCenterChangedSignal();
}

void ResourceCenter::SetSecondaryFocus(FocusType focus)
{
    m_secondary = focus;
    ResourceCenterChangedSignal();
}

void ResourceCenter::ResourceCenterResetTargetMaxUnpairedMeters(MeterType meter_type/* = INVALID_METER_TYPE*/)
{
    if (meter_type == INVALID_METER_TYPE || meter_type == METER_TARGET_CONSTRUCTION) {
        GetMeter(METER_TARGET_CONSTRUCTION)->ResetCurrent();
        GetMeter(METER_TARGET_CONSTRUCTION)->AddToCurrent(10.0);// TODO: replace with effect as part of species
    }

    std::vector<MeterType> res_meter_types;
    res_meter_types.push_back(METER_TARGET_FARMING);
    res_meter_types.push_back(METER_TARGET_MINING);
    res_meter_types.push_back(METER_TARGET_INDUSTRY);
    res_meter_types.push_back(METER_TARGET_RESEARCH);
    res_meter_types.push_back(METER_TARGET_TRADE);

    // all meters matching parameter meter_type should be adjusted, depending on focus
    for (unsigned int i = 0; i < res_meter_types.size(); ++i) {
        const MeterType CUR_METER_TYPE = res_meter_types[i];

        if (meter_type == INVALID_METER_TYPE || meter_type == CUR_METER_TYPE) {
            Meter* meter = GetMeter(CUR_METER_TYPE);
            meter->ResetCurrent();

            if (m_primary == MeterToFocus(CUR_METER_TYPE))
                meter->AddToCurrent(PRIMARY_FOCUS_BONUS);
            else if (m_primary == FOCUS_BALANCED)
                meter->AddToCurrent(PRIMARY_BALANCED_BONUS);

            if (m_secondary == MeterToFocus(CUR_METER_TYPE))
                meter->AddToCurrent(SECONDARY_FOCUS_BONUS);
            else if (m_secondary == FOCUS_BALANCED)
                meter->AddToCurrent(SECONDARY_BALANCED_BONUS);
        }
    }
}

void ResourceCenter::ResourceCenterPopGrowthProductionResearchPhase()
{
    GetMeter(METER_CONSTRUCTION)->AddToCurrent(1.0);
    GetMeter(METER_FARMING)->AddToCurrent(1.0);
    GetMeter(METER_INDUSTRY)->AddToCurrent(1.0);
    GetMeter(METER_MINING)->AddToCurrent(1.0);
    GetMeter(METER_RESEARCH)->AddToCurrent(1.0);
    GetMeter(METER_TRADE)->AddToCurrent(1.0);
}

void ResourceCenter::ResourceCenterClampMeters()
{
    GetMeter(METER_FARMING)->ClampCurrentToRange();
    GetMeter(METER_INDUSTRY)->ClampCurrentToRange();
    GetMeter(METER_MINING)->ClampCurrentToRange();
    GetMeter(METER_RESEARCH)->ClampCurrentToRange();
    GetMeter(METER_TRADE)->ClampCurrentToRange();
    GetMeter(METER_CONSTRUCTION)->ClampCurrentToRange();

    GetMeter(METER_TARGET_FARMING)->ClampCurrentToRange();
    GetMeter(METER_TARGET_INDUSTRY)->ClampCurrentToRange();
    GetMeter(METER_TARGET_MINING)->ClampCurrentToRange();
    GetMeter(METER_TARGET_RESEARCH)->ClampCurrentToRange();
    GetMeter(METER_TARGET_TRADE)->ClampCurrentToRange();
    GetMeter(METER_TARGET_CONSTRUCTION)->ClampCurrentToRange();
}

void ResourceCenter::Reset()
{
    m_primary = INVALID_FOCUS_TYPE;
    m_secondary = INVALID_FOCUS_TYPE;

    GetMeter(METER_FARMING)->Reset();
    GetMeter(METER_INDUSTRY)->Reset();
    GetMeter(METER_MINING)->Reset();
    GetMeter(METER_RESEARCH)->Reset();
    GetMeter(METER_TRADE)->Reset();
    GetMeter(METER_CONSTRUCTION)->Reset();

    GetMeter(METER_TARGET_FARMING)->Reset();
    GetMeter(METER_TARGET_INDUSTRY)->Reset();
    GetMeter(METER_TARGET_MINING)->Reset();
    GetMeter(METER_TARGET_RESEARCH)->Reset();
    GetMeter(METER_TARGET_TRADE)->Reset();
    GetMeter(METER_TARGET_CONSTRUCTION)->Reset();
}
