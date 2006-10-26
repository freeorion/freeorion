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
#include "../util/XMLDoc.h"
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
        double construction_delta =
            (construction.Current() + 1) * ((construction.Max() - construction.Current()) / (construction.Max() + 1)) * (pop.Current() * 10.0) * 0.01;
        construction.AdjustCurrent(construction_delta);

        // update the resource meters
        farming.AdjustCurrent(construction.Current() / (10.0 + farming.Current()));
        industry.AdjustCurrent(construction.Current() / (10.0 + industry.Current()));
        mining.AdjustCurrent(construction.Current() / (10.0 + mining.Current()));
        research.AdjustCurrent(construction.Current() / (10.0 + research.Current()));
        trade.AdjustCurrent(construction.Current() / (10.0 + trade.Current()));
    }

}

ResourceCenter::ResourceCenter(const Meter& pop) : 
    m_pop(pop)
{
    Reset();
}

ResourceCenter::ResourceCenter(const XMLElement& elem, const Meter& pop) : 
    m_primary(FOCUS_UNKNOWN),
    m_secondary(FOCUS_UNKNOWN),
    m_pop(pop)
{
    if (elem.Tag() != "ResourceCenter")
        throw std::invalid_argument("Attempted to construct a ResourceCenter from an XMLElement that had a tag other than \"ResourceCenter\"");

    try {
        UniverseObject::Visibility vis = UniverseObject::Visibility(lexical_cast<int>(elem.Child("vis").Text()));
        if (vis == UniverseObject::FULL_VISIBILITY) {
            m_primary = lexical_cast<FocusType>(elem.Child("m_primary").Text());
            m_secondary = lexical_cast<FocusType>(elem.Child("m_secondary").Text());
            m_farming = Meter(elem.Child("m_farming").Child("Meter"));
            m_industry = Meter(elem.Child("m_industry").Child("Meter"));
            m_mining = Meter(elem.Child("m_mining").Child("Meter"));
            m_research = Meter(elem.Child("m_research").Child("Meter"));
            m_trade = Meter(elem.Child("m_trade").Child("Meter"));
            m_construction = Meter(elem.Child("m_construction").Child("Meter"));
        }
    } catch (const boost::bad_lexical_cast& e) {
        Logger().debugStream() << "Caught boost::bad_lexical_cast in ResourceCenter::ResourceCenter(); bad XMLElement was:";
        std::stringstream osstream;
        elem.WriteElement(osstream);
        Logger().debugStream() << "\n" << osstream.str();
        throw;
    }
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
    return m_pop.Current() / 10.0 * m_farming.Current();
}

double ResourceCenter::IndustryPoints() const
{
    return m_pop.Current() / 10.0 * m_industry.Current();
}

double ResourceCenter::MiningPoints() const
{
    return m_pop.Current() / 10.0 * m_mining.Current();
}

double ResourceCenter::ResearchPoints() const
{
    return m_pop.Current() / 10.0 * m_research.Current();
}

double ResourceCenter::TradePoints() const
{
    return m_pop.Current() / 10.0 * m_trade.Current();
}

double ResourceCenter::ProjectedCurrent(MeterType type) const
{
    Meter construction = m_construction, farming = m_farming, industry = m_industry, mining = m_mining, research = m_research, trade = m_trade;
    Growth(construction, farming, industry, mining, research, trade, m_pop);
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

XMLElement ResourceCenter::XMLEncode(UniverseObject::Visibility vis) const
{
    // partial encode version -- no current production info
    using boost::lexical_cast;
    using std::string;

    XMLElement retval("ResourceCenter");
    retval.AppendChild(XMLElement("vis", lexical_cast<string>(vis)));
    if (vis == UniverseObject::FULL_VISIBILITY) {
        retval.AppendChild(XMLElement("m_primary", lexical_cast<string>(m_primary)));
        retval.AppendChild(XMLElement("m_secondary", lexical_cast<string>(m_secondary)));
        retval.AppendChild(XMLElement("m_farming", m_farming.XMLEncode()));
        retval.AppendChild(XMLElement("m_industry", m_industry.XMLEncode()));
        retval.AppendChild(XMLElement("m_mining", m_mining.XMLEncode()));
        retval.AppendChild(XMLElement("m_research", m_research.XMLEncode()));
        retval.AppendChild(XMLElement("m_trade", m_trade.XMLEncode()));
        retval.AppendChild(XMLElement("m_construction", m_construction.XMLEncode()));
    }
    return retval;
}


void ResourceCenter::SetPrimaryFocus(FocusType focus)
{
    for (MeterType i = METER_FARMING; i <= METER_MINING; i = MeterType(i + 1)) {
        double old_max_mods = 0.0;
        if (m_primary == MeterToFocus(i))
            old_max_mods = ProductionDataTables()["FocusMods"][0][0];
        else if (m_primary == FOCUS_BALANCED)
            old_max_mods = ProductionDataTables()["FocusMods"][2][0];
        double new_max_mods = 0.0;
        if (focus == MeterToFocus(i))
            new_max_mods = ProductionDataTables()["FocusMods"][0][0];
        else if (focus == FOCUS_BALANCED)
            new_max_mods = ProductionDataTables()["FocusMods"][2][0];
        Meter* meter = GetMeter(i);
        assert(meter);
        meter->SetMax(meter->Max() + new_max_mods - old_max_mods);
    }
    m_primary = focus;
    ResourceCenterChangedSignal();
}

void ResourceCenter::SetSecondaryFocus(FocusType focus)
{
    for (MeterType i = METER_FARMING; i <= METER_MINING; i = MeterType(i + 1)) {
        double old_max_mods = 0.0;
        if (m_secondary == MeterToFocus(i))
            old_max_mods = ProductionDataTables()["FocusMods"][1][0];
        else if (m_secondary == FOCUS_BALANCED)
            old_max_mods = ProductionDataTables()["FocusMods"][3][0];
        double new_max_mods = 0.0;
        if (focus == MeterToFocus(i))
            new_max_mods = ProductionDataTables()["FocusMods"][1][0];
        else if (focus == FOCUS_BALANCED)
            new_max_mods = ProductionDataTables()["FocusMods"][3][0];
        Meter* meter = GetMeter(i);
        assert(meter);
        meter->SetMax(meter->Max() + new_max_mods - old_max_mods);
    }
    m_secondary = focus;
    ResourceCenterChangedSignal();
}

void ResourceCenter::AdjustMaxMeters()
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
    Growth(m_construction, m_farming, m_industry, m_mining, m_research, m_trade, m_pop);
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
