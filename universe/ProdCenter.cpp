#include "ProdCenter.h"

#include "../util/AppInterface.h"
#include "../util/DataTable.h"
#include "../Empire/Empire.h"
#include "Fleet.h"
#include "Planet.h"
#include "System.h"
#include "XMLDoc.h"

#include <boost/lexical_cast.hpp>

#include <stdexcept>

using boost::lexical_cast;

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

    // v0.2 only
    int Cost(ProdCenter::BuildType item)
    {
        switch (item) {
        case ProdCenter::DEF_BASE:      return 200;
        case ProdCenter::SCOUT:         return 50;
        case ProdCenter::COLONY_SHIP:   return 250;
        case ProdCenter::MARKI:         return 100;
        case ProdCenter::MARKII:        return 200;
        case ProdCenter::MARKIII:       return 375;
        case ProdCenter::MARKIV:        return 700;
        default:                        return 0;
        }
        return 0;
    }

    double MaxMeterAdjustment(FocusType meter, FocusType old_focus, FocusType new_focus, bool primary)
    {
        double primary_specialized_factor = ProductionDataTables()["FocusMods"][0][0];
        double secondary_specialized_factor = ProductionDataTables()["FocusMods"][1][0];
        double primary_balanced_factor = ProductionDataTables()["FocusMods"][2][0];
        double secondary_balanced_factor = ProductionDataTables()["FocusMods"][3][0];
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
        if (const Planet* planet = dynamic_cast<const Planet*>(object)) {
            retval = PlanetDataTables()["PlanetEnvFarmingMod"][0][planet->Environment()];
        }
        return retval;
    }

    double MaxIndustryModFromObject(const UniverseObject* object)
    {
        double retval = 0.0;
        if (const Planet* planet = dynamic_cast<const Planet*>(object)) {
            retval = PlanetDataTables()["PlanetSizeIndustryMod"][0][planet->Environment()];
        }
        return retval;
    }
}

ProdCenter::ProdCenter(const Meter& pop, UniverseObject* object) : 
    m_primary(FOCUS_BALANCED),
    m_secondary(FOCUS_BALANCED),
    m_pop(pop),
    m_object(object),
    m_available_minerals(0.0),
    m_currently_building(NOT_BUILDING, ""),
    m_rollover(0)
{
    assert(m_object);

    double balanced_balanced_max = ProductionDataTables()["FocusMods"][2][0] + ProductionDataTables()["FocusMods"][3][0];
    m_farming.SetMax(balanced_balanced_max);
    m_industry.SetMax(balanced_balanced_max);
    m_mining.SetMax(balanced_balanced_max);
    m_research.SetMax(balanced_balanced_max);
    m_trade.SetMax(balanced_balanced_max);
}

ProdCenter::ProdCenter(const GG::XMLElement& elem, const Meter& pop, UniverseObject* object) : 
    m_primary(FOCUS_BALANCED),
    m_secondary(FOCUS_BALANCED),
    m_pop(pop),
    m_object(object),
    m_available_minerals(0.0),
    m_currently_building(NOT_BUILDING, ""),
    m_rollover(0)
{
    assert(m_object);

    if (elem.Tag() != "ProdCenter")
        throw std::invalid_argument("Attempted to construct a ProdCenter from an XMLElement that had a tag other than \"ProdCenter\"");

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
            m_currently_building.first = BuildType(lexical_cast<int>(elem.Child("m_currently_building").Child("first").Text()));
            m_currently_building.second = elem.Child("m_currently_building").Child("second").Text();
            m_rollover = lexical_cast<double>(elem.Child("m_rollover").Text());
            m_available_minerals = lexical_cast<double>(elem.Child("m_available_minerals").Text());
        }
    } catch (const boost::bad_lexical_cast& e) {
        Logger().debugStream() << "Caught boost::bad_lexical_cast in ProdCenter::ProdCenter(); bad XMLElement was:";
        std::stringstream osstream;
        elem.WriteElement(osstream);
        Logger().debugStream() << "\n" << osstream.str();
        throw;
    }
}

ProdCenter::~ProdCenter()
{
}

double ProdCenter::FarmingPoints() const
{
    return m_pop.Current() / 10.0 * m_farming.Current();
}

double ProdCenter::IndustryPoints() const
{
    return m_pop.Current() / 10.0 * m_industry.Current();
}

double ProdCenter::MiningPoints() const
{
    return m_pop.Current() / 10.0 * m_mining.Current();
}

double ProdCenter::ResearchPoints() const
{
    return m_pop.Current() / 10.0 * m_research.Current();
}

double ProdCenter::TradePoints() const
{
    return m_pop.Current() / 10.0 * m_trade.Current();
}

double ProdCenter::PercentComplete() const
{
    double cost = ItemBuildCost();
    return (cost ? (m_rollover / cost) : 0.0);
}

double ProdCenter::ItemBuildCost() const
{
    return Cost(m_currently_building.first);
}

double ProdCenter::ProductionPoints() const
{
    return std::min(IndustryPoints(), AvailableMinerals());
}

double ProdCenter::ProductionPointsMax() const
{
    return IndustryPoints();
}

GG::XMLElement ProdCenter::XMLEncode(UniverseObject::Visibility vis) const
{
    // partial encode version -- no current production info
    using GG::XMLElement;
    using boost::lexical_cast;
    using std::string;

    XMLElement retval("ProdCenter");
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
        retval.AppendChild(XMLElement("m_currently_building"));
        retval.LastChild().AppendChild(XMLElement("first", lexical_cast<string>(m_currently_building.first)));
        retval.LastChild().AppendChild(XMLElement("second", m_currently_building.second));
        retval.AppendChild(XMLElement("m_rollover", lexical_cast<string>(m_rollover)));
        retval.AppendChild(XMLElement("m_available_minerals", lexical_cast<string>(m_available_minerals)));
    }
    return retval;
}


void ProdCenter::SetPrimaryFocus(FocusType focus)
{
    m_farming.SetMax(m_farming.Max() + MaxMeterAdjustment(FOCUS_FARMING, m_primary, focus, true));
    m_industry.SetMax(m_industry.Max() + MaxMeterAdjustment(FOCUS_INDUSTRY, m_primary, focus, true));
    m_mining.SetMax(m_mining.Max() + MaxMeterAdjustment(FOCUS_MINING, m_primary, focus, true));
    m_research.SetMax(m_research.Max() + MaxMeterAdjustment(FOCUS_RESEARCH, m_primary, focus, true));
    m_trade.SetMax(m_trade.Max() + MaxMeterAdjustment(FOCUS_TRADE, m_primary, focus, true));
    m_primary = focus;
    m_prod_changed_sig();
}

void ProdCenter::SetSecondaryFocus(FocusType focus)
{
    m_farming.SetMax(m_farming.Max() + MaxMeterAdjustment(FOCUS_FARMING, m_secondary, focus, false));
    m_industry.SetMax(m_industry.Max() + MaxMeterAdjustment(FOCUS_INDUSTRY, m_secondary, focus, false));
    m_mining.SetMax(m_mining.Max() + MaxMeterAdjustment(FOCUS_MINING, m_secondary, focus, false));
    m_research.SetMax(m_research.Max() + MaxMeterAdjustment(FOCUS_RESEARCH, m_secondary, focus, false));
    m_trade.SetMax(m_trade.Max() + MaxMeterAdjustment(FOCUS_TRADE, m_secondary, focus, false));
    m_secondary = focus;
    m_prod_changed_sig();
}

void ProdCenter::SetProduction(ProdCenter::BuildType type, const std::string& name)
{
    m_currently_building = std::make_pair(type, name);
    m_prod_changed_sig();
}

void ProdCenter::AdjustMaxMeters()
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

void ProdCenter::PopGrowthProductionResearchPhase()
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
  
    // v0.3 ONLY
    Empire* empire = Empires().Lookup(*m_object->Owners().begin());
    Planet* planet = dynamic_cast<Planet*>(m_object);
    if (m_currently_building.first == DEF_BASE && planet) {
        // for v0.3 we hard-code values for cost of bases
        int new_bases = UpdateBuildProgress( 200 );
    
        if ( new_bases > 0 )
        {
            // add base
            planet->AdjustDefBases( new_bases );

            // add sitrep
            SitRepEntry *p_entry = CreateBaseBuiltSitRep( m_object->SystemID(), m_object->ID() );
            empire->AddSitRepEntry( p_entry );
        }
        // V0.3 only - we would have a better way to know we're building different ships
        // for now enumerate through the ones we can build
    } else if ( m_currently_building.first == ProdCenter::SCOUT ) {
        UpdateShipBuildProgress( empire, ShipDesign::SCOUT );
    } else if ( m_currently_building.first == ProdCenter::COLONY_SHIP ) {
        UpdateShipBuildProgress( empire, ShipDesign::COLONY );
    } else if ( m_currently_building.first == ProdCenter::MARKI ) {
        UpdateShipBuildProgress( empire, ShipDesign::MARK1 );
    } else if ( m_currently_building.first == ProdCenter::MARKII ) {
        UpdateShipBuildProgress( empire, ShipDesign::MARK2 );
    } else if ( m_currently_building.first == ProdCenter::MARKIII ) {
        UpdateShipBuildProgress( empire, ShipDesign::MARK3 );
    } else if ( m_currently_building.first == ProdCenter::MARKIV ) {
        UpdateShipBuildProgress( empire, ShipDesign::MARK4 );
    } else if ( m_currently_building.first == ProdCenter::BUILDING ) {
        // TODO
    }
    // v0.3 ONLY
}


int ProdCenter::UpdateBuildProgress(int item_cost)
{
    double total_build_points =  m_rollover + ProductionPoints();
    int new_items = static_cast<int>(total_build_points / item_cost);
    m_rollover = total_build_points - (new_items * item_cost);
    return new_items;
}


void ProdCenter::UpdateShipBuildProgress(Empire *empire, ShipDesign::V02DesignID design_id)
{
    Universe* universe = &GetUniverse();
    ShipDesign ship_design;

    // get ship design we're trying to build
    if (empire->CopyShipDesign((int)design_id, ship_design))
    {
        int new_ships = UpdateBuildProgress(ship_design.cost);

        if (new_ships > 0)
        {
            System* system = m_object->GetSystem();

            // create new fleet with new ship
            Fleet* new_fleet = new Fleet("", system->X(), system->Y(), empire->EmpireID());
            int fleet_id = universe->Insert(new_fleet);
  
            // set name
            // TODO: What is the mechanism for determining new fleet name?
            std::string fleet_name("New fleet ");
            fleet_name += boost::lexical_cast<std::string>(fleet_id);
            new_fleet->Rename(fleet_name);

            // insert fleet around this system
            system->Insert(new_fleet);
  
            // add new ship (s)
            for (int i = 0; i < new_ships; ++i)
            {
                Ship *new_ship = new Ship(empire->EmpireID(), (int)design_id);
                int ship_id = universe->Insert(new_ship);

                std::string ship_name(ship_design.name);
                ship_name += boost::lexical_cast<std::string>(ship_id);
                new_ship->Rename(ship_name);

                new_fleet->AddShip(ship_id);

                // add sitrep
                SitRepEntry *p_entry = CreateShipBuiltSitRep(ship_id, m_object->ID());
                empire->AddSitRepEntry(p_entry);
            }
        }
    }
}


