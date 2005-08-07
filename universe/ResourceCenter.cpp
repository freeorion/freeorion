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
#include "XMLDoc.h"

#include <boost/lexical_cast.hpp>

#include <stdexcept>

using boost::lexical_cast;

#if defined(WIN32) && defined(_MSC_VER)
#ifdef _DEBUG
// Fix for evil M$ compile bug: fatal error C1055: compiler limit : out of keys
#undef assert
#define assert(exp) (void)( (exp) || (_assert(#exp, __FILE__, -1), 0) )
#endif 
#endif

namespace {
    DataTableMap& ProductionDataTables()
    {
        static DataTableMap map;
        if (map.empty()) {
            LoadDataTables("default/production_tables.txt", map);
        }
        return map;
    }

    DataTableMap& PlanetDataTables()
    {
        static DataTableMap map;
        if (map.empty()) {
            LoadDataTables("default/planet_tables.txt", map);
        }
        return map;
    }

    double MaxMeterAdjustment(FocusType meter, FocusType old_focus, FocusType new_focus, bool primary)
    {
        double primary_specialized_factor = ProductionDataTables()["FocusMods"][0][0];
        double secondary_specialized_factor = ProductionDataTables()["FocusMods"][1][0];
        double primary_balanced_factor = ProductionDataTables()["FocusMods"][2][0];
        double secondary_balanced_factor = ProductionDataTables()["FocusMods"][3][0];
        if (old_focus == FOCUS_UNKNOWN)
            old_focus = FOCUS_BALANCED;
        if (old_focus != new_focus) {
            if (old_focus == FOCUS_BALANCED && new_focus == meter) {
                return primary ? (primary_specialized_factor - primary_balanced_factor) : (secondary_specialized_factor - secondary_balanced_factor);
            } else if (old_focus == FOCUS_BALANCED) {
                return primary ? -primary_balanced_factor : -secondary_balanced_factor;
            } else if (old_focus == meter && new_focus == FOCUS_BALANCED) {
                return primary ? (primary_balanced_factor - primary_specialized_factor) : (secondary_balanced_factor - secondary_specialized_factor);
            } else if (new_focus == FOCUS_BALANCED) {
                return primary ? primary_balanced_factor : secondary_balanced_factor;
            }
        }
        return 0.0;
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

    bool temp_header_bool = RecordHeaderFile(ResourceCenterRevision());
    bool temp_source_bool = RecordSourceFile("$RCSfile$", "$Revision$");
}

ResourceCenter::ResourceCenter(const Meter& pop, UniverseObject* object) : 
    m_pop(pop),
    m_object(object)
{
    assert(m_object);
    Reset();
}

ResourceCenter::ResourceCenter(const GG::XMLElement& elem, const Meter& pop, UniverseObject* object) : 
    m_primary(FOCUS_UNKNOWN),
    m_secondary(FOCUS_UNKNOWN),
    m_pop(pop),
    m_object(object)
{
    assert(m_object);

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

GG::XMLElement ResourceCenter::XMLEncode(UniverseObject::Visibility vis) const
{
    // partial encode version -- no current production info
    using GG::XMLElement;
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
    m_farming.SetMax(m_farming.Max() + MaxMeterAdjustment(FOCUS_FARMING, m_primary, focus, true));
    m_industry.SetMax(m_industry.Max() + MaxMeterAdjustment(FOCUS_INDUSTRY, m_primary, focus, true));
    m_mining.SetMax(m_mining.Max() + MaxMeterAdjustment(FOCUS_MINING, m_primary, focus, true));
    m_research.SetMax(m_research.Max() + MaxMeterAdjustment(FOCUS_RESEARCH, m_primary, focus, true));
    m_trade.SetMax(m_trade.Max() + MaxMeterAdjustment(FOCUS_TRADE, m_primary, focus, true));
    m_primary = focus;
    ResourceCenterChangedSignal();
}

void ResourceCenter::SetSecondaryFocus(FocusType focus)
{
    m_farming.SetMax(m_farming.Max() + MaxMeterAdjustment(FOCUS_FARMING, m_secondary, focus, false));
    m_industry.SetMax(m_industry.Max() + MaxMeterAdjustment(FOCUS_INDUSTRY, m_secondary, focus, false));
    m_mining.SetMax(m_mining.Max() + MaxMeterAdjustment(FOCUS_MINING, m_secondary, focus, false));
    m_research.SetMax(m_research.Max() + MaxMeterAdjustment(FOCUS_RESEARCH, m_secondary, focus, false));
    m_trade.SetMax(m_trade.Max() + MaxMeterAdjustment(FOCUS_TRADE, m_secondary, focus, false));
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
    m_farming.AdjustMax(MaxFarmingModFromObject(m_object));
    m_industry.AdjustMax(MaxIndustryModFromObject(m_object));
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
    // update the construction meter
    double construction_delta =
        (m_construction.Current() + 1) * ((m_construction.Max() - m_construction.Current()) / (m_construction.Max() + 1)) * (m_pop.Current() * 10.0) * 0.01;
    m_construction.AdjustCurrent(construction_delta);

    // update the resource meters
    m_farming.AdjustCurrent(m_construction.Current() / (10.0 + m_farming.Current()));
    m_industry.AdjustCurrent(m_construction.Current() / (10.0 + m_industry.Current()));
    m_mining.AdjustCurrent(m_construction.Current() / (10.0 + m_mining.Current()));
    m_research.AdjustCurrent(m_construction.Current() / (10.0 + m_research.Current()));
    m_trade.AdjustCurrent(m_construction.Current() / (10.0 + m_trade.Current()));
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
