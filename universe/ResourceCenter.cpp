#include "ResourceCenter.h"

#include "../util/AppInterface.h"
#include "../universe/Building.h"
#include "../util/DataTable.h"
#include "../Empire/Empire.h"
#include "Fleet.h"
#include "../util/MultiplayerCommon.h"
#include "Planet.h"
#include "../universe/ShipDesign.h"
#include "System.h"
#include "../util/OptionsDB.h"

#include <boost/lexical_cast.hpp>

#include <stdexcept>

using boost::lexical_cast;

namespace {
    DataTableMap& ProductionDataTables()
    {
        static DataTableMap map;
        if (map.empty()) {
            std::string settings_dir = GetOptionsDB().Get<std::string>("settings-dir");
            if (!settings_dir.empty() && settings_dir[settings_dir.size() - 1] != '/')
                settings_dir += '/';

            LoadDataTables(settings_dir + "production_tables.txt", map);
        }
        return map;
    }

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

    double MaxFarmingModFromObject(const UniverseObject* object)
    {
        double retval = 0.0;
        if (const Planet* planet = universe_object_cast<const Planet*>(object)) {
            retval = PlanetDataTables()["PlanetEnvFarmingMod"][0][planet->Environment()];
        }
        return retval;
    }

    double MaxIndustryModFromObject(const UniverseObject* object)
    {
        double retval = 0.0;
        if (const Planet* planet = universe_object_cast<const Planet*>(object)) {
            retval = PlanetDataTables()["PlanetSizeIndustryMod"][0][planet->Environment()];
        }
        return retval;
    }

    void Growth(Meter& construction, Meter& farming, Meter& industry, Meter& mining, Meter& research, Meter& trade, const Meter& pop)
    {
        // update the construction meter
        double delta = (construction.Current() + 1) * ((construction.Max() - construction.Current()) / (construction.Max() + 1)) * (pop.Current() * 10.0) * 0.01;
        double new_cur = std::min(construction.Max(), construction.Current() + delta);
        construction.SetCurrent(new_cur);

        // update the resource meters
        delta = construction.Current() / (10.0 + farming.Current());
        new_cur = std::min(farming.Max(), farming.Current() + delta);
        farming.SetCurrent(new_cur);

        delta = construction.Current() / (10.0 + industry.Current());
        new_cur = std::min(industry.Max(), industry.Current() + delta);
        industry.SetCurrent(new_cur);

        delta = construction.Current() / (10.0 + mining.Current());
        new_cur = std::min(mining.Max(), mining.Current() + delta);
        mining.SetCurrent(new_cur);

        delta = construction.Current() / (10.0 + research.Current());
        new_cur = std::min(research.Max(), research.Current() + delta);
        research.SetCurrent(new_cur);

        delta = construction.Current() / (10.0 + trade.Current());
        new_cur = std::min(trade.Max(), trade.Current() + delta);
        trade.SetCurrent(new_cur);
    }
}

ResourceCenter::ResourceCenter() : 
    m_primary(FOCUS_UNKNOWN),
    m_secondary(FOCUS_UNKNOWN),
    m_pop(0)
{}

ResourceCenter::ResourceCenter(const Meter& pop) : 
    m_pop(&pop)
{
    Reset();
}

ResourceCenter::~ResourceCenter()
{
}

const Meter* ResourceCenter::GetMeter(MeterType type) const
{
    switch (type) {
    case METER_FARMING: return &m_farming;
    case METER_INDUSTRY: return &m_industry;
    case METER_RESEARCH: return &m_research;
    case METER_TRADE: return &m_trade;
    case METER_MINING: return &m_mining;
    case METER_CONSTRUCTION: return &m_construction;
    default: return 0;
    }
}

double ResourceCenter::FarmingPoints() const
{
    return m_pop->Current() / 10.0 * m_farming.Current();
}

double ResourceCenter::IndustryPoints() const
{
    return m_pop->Current() / 10.0 * m_industry.Current();
}

double ResourceCenter::MiningPoints() const
{
    return m_pop->Current() / 10.0 * m_mining.Current();
}

double ResourceCenter::ResearchPoints() const
{
    return m_pop->Current() / 10.0 * m_research.Current();
}

double ResourceCenter::TradePoints() const
{
    return m_pop->Current() / 10.0 * m_trade.Current();
}

double ResourceCenter::ProjectedCurrent(MeterType type) const
{
    Meter construction = m_construction;
    Meter farming = Meter(m_farming.Current(), m_farming.Max());
    Meter mining = Meter(m_mining.Current(), m_mining.Max());
    Meter industry = Meter(m_industry.Current(), m_industry.Max());
    Meter research = Meter(m_research.Current(), m_research.Max());
    Meter trade = Meter(m_trade.Current(), m_trade.Max());

    Growth(construction, farming, industry, mining, research, trade, *m_pop);
    switch (type) {
    case METER_FARMING: return farming.Current();
    case METER_INDUSTRY: return industry.Current();
    case METER_RESEARCH: return research.Current();
    case METER_TRADE: return trade.Current();
    case METER_MINING: return mining.Current();
    case METER_CONSTRUCTION: return construction.Current();
    default:
        assert(0);
        return 0.0;
    }
}

double ResourceCenter::ProjectedFarmingPoints() const
{
    return m_pop->Current() / 10.0 * ProjectedCurrent(METER_FARMING);
}

double ResourceCenter::ProjectedIndustryPoints() const
{
    return m_pop->Current() / 10.0 * ProjectedCurrent(METER_INDUSTRY);
}

double ResourceCenter::ProjectedMiningPoints() const
{
    return m_pop->Current() / 10.0 * ProjectedCurrent(METER_MINING);
}

double ResourceCenter::ProjectedResearchPoints() const
{
    return m_pop->Current() / 10.0 * ProjectedCurrent(METER_RESEARCH);
}

double ResourceCenter::ProjectedTradePoints() const
{
    return m_pop->Current() / 10.0 * ProjectedCurrent(METER_TRADE);
}

Meter* ResourceCenter::GetMeter(MeterType type)
{
    switch (type) {
    case METER_FARMING: return &m_farming;
    case METER_INDUSTRY: return &m_industry;
    case METER_RESEARCH: return &m_research;
    case METER_TRADE: return &m_trade;
    case METER_MINING: return &m_mining;
    case METER_CONSTRUCTION: return &m_construction;
    default: return 0;
    }
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

void ResourceCenter::ApplyUniverseTableMaxMeterAdjustments()
{
    // determine meter maxes; they should have been previously reset to 0
    double primary_specialized_factor = ProductionDataTables()["FocusMods"][0][0];
    double secondary_specialized_factor = ProductionDataTables()["FocusMods"][1][0];
    double primary_balanced_factor = ProductionDataTables()["FocusMods"][2][0];
    double secondary_balanced_factor = ProductionDataTables()["FocusMods"][3][0];
    m_construction.AdjustMax(20.0); // default construction max is 20
    UniverseObject* object = GetObjectSignal();
    assert(object);
    m_farming.AdjustMax(MaxFarmingModFromObject(object));
    m_industry.AdjustMax(MaxIndustryModFromObject(object));
    switch (m_primary) {
    case FOCUS_BALANCED:
        m_farming.AdjustMax(primary_balanced_factor);
        m_industry.AdjustMax(primary_balanced_factor);
        m_mining.AdjustMax(primary_balanced_factor);
        m_research.AdjustMax(primary_balanced_factor);
        m_trade.AdjustMax(primary_balanced_factor);
        break;

    case FOCUS_FARMING:  m_farming.AdjustMax(primary_specialized_factor); break;
    case FOCUS_INDUSTRY: m_industry.AdjustMax(primary_specialized_factor); break;
    case FOCUS_MINING:   m_mining.AdjustMax(primary_specialized_factor); break;
    case FOCUS_RESEARCH: m_research.AdjustMax(primary_specialized_factor ); break;
    case FOCUS_TRADE:    m_trade.AdjustMax(primary_specialized_factor); break;

    default:             break;
    }
    switch (m_secondary) {
    case FOCUS_BALANCED: 
        m_farming.AdjustMax(secondary_balanced_factor);
        m_industry.AdjustMax(secondary_balanced_factor);
        m_mining.AdjustMax(secondary_balanced_factor);
        m_research.AdjustMax(secondary_balanced_factor);
        m_trade.AdjustMax(secondary_balanced_factor);
        break;

    case FOCUS_FARMING:  m_farming.AdjustMax(secondary_specialized_factor); break;
    case FOCUS_INDUSTRY: m_industry.AdjustMax(secondary_specialized_factor); break;
    case FOCUS_MINING:   m_mining.AdjustMax(secondary_specialized_factor); break;
    case FOCUS_RESEARCH: m_research.AdjustMax(secondary_specialized_factor); break;
    case FOCUS_TRADE:    m_trade.AdjustMax(secondary_specialized_factor); break;

    default:             break;
    }
}

void ResourceCenter::PopGrowthProductionResearchPhase()
{
    Growth(m_construction, m_farming, m_industry, m_mining, m_research, m_trade, *m_pop);
}


void ResourceCenter::Reset()
{
    m_primary = FOCUS_UNKNOWN;
    m_secondary = FOCUS_UNKNOWN;

    m_farming = Meter();
    m_industry = Meter();
    m_mining = Meter();
    m_research = Meter();
    m_trade = Meter();
    m_construction = Meter();

    double balanced_balanced_max = ProductionDataTables()["FocusMods"][2][0] + ProductionDataTables()["FocusMods"][3][0];
    m_farming.SetMax(balanced_balanced_max);
    m_industry.SetMax(balanced_balanced_max);
    m_mining.SetMax(balanced_balanced_max);
    m_research.SetMax(balanced_balanced_max);
    m_trade.SetMax(balanced_balanced_max);
}
