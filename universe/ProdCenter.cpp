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

namespace {
    DataTableMap& ProductionDataTables()
    {
        static DataTableMap map;
        if (map.empty()) {
            LoadDataTables("default/production_tables.txt", map);
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
        }
        return 0;
    }
}

using boost::lexical_cast;

ProdCenter::ProdCenter() : 
   m_primary(BALANCED),
   m_secondary(BALANCED),
   m_workforce(0.0),
   m_max_workforce(0.0),
   m_planet_type(PT_TERRAN),
   m_currently_building(NOT_BUILDING),
   m_rollover(0),
   m_available_minerals(0.0)
{
}

ProdCenter::ProdCenter(const GG::XMLElement& elem) : 
   m_primary(BALANCED),
   m_secondary(BALANCED),
   m_workforce(0.0),
   m_max_workforce(0.0),
   m_planet_type(PT_TERRAN),
   m_currently_building(NOT_BUILDING),
   m_rollover(0),
   m_available_minerals(0.0)
{
    if (elem.Tag() != "ProdCenter")
        throw std::invalid_argument("Attempted to construct a ProdCenter from an XMLElement that had a tag other than \"ProdCenter\"");

    try {
        m_max_workforce = lexical_cast<double>(elem.Child("m_max_workforce").Text());

        UniverseObject::Visibility vis = UniverseObject::Visibility(lexical_cast<int>(elem.Child("vis").Text()));
        if (vis == UniverseObject::FULL_VISIBILITY) {
            m_primary = FocusType(lexical_cast<int>(elem.Child("m_primary").Text()));
            m_secondary = FocusType(lexical_cast<int>(elem.Child("m_secondary").Text()));
            m_workforce = lexical_cast<double>(elem.Child("m_workforce").Text());
            m_planet_type = lexical_cast<PlanetType>(elem.Child("m_planet_type").Text());
            m_currently_building = BuildType(lexical_cast<int>(elem.Child("m_currently_building").Text()));
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
    int modifier = std::max(0,
                            (m_primary == FOCUS_UNKNOWN ? 0 : ProductionDataTables()["PrimaryFocusMod"][m_primary - 1][0]) +
                            (m_secondary == FOCUS_UNKNOWN ? 0 : ProductionDataTables()["SecondaryFocusMod"][m_secondary - 1][0]) +
                            ProductionDataTables()["EnvironmentProductionMod"][m_planet_type][0]);
    return m_workforce * modifier;
}

double ProdCenter::IndustryPoints() const
{
    return  m_workforce 
          *(  ((m_primary  ==FOCUS_UNKNOWN)?0:ProductionDataTables()["PrimaryFocusMod"][m_primary - 1][2] )
            + ((m_secondary==FOCUS_UNKNOWN)?0:ProductionDataTables()["SecondaryFocusMod"][m_secondary - 1][2]));
}

double ProdCenter::MiningPoints() const
{
    return  m_workforce 
          *(  ((m_primary  ==FOCUS_UNKNOWN)?0:ProductionDataTables()["PrimaryFocusMod"][m_primary - 1][1]) 
            + ((m_secondary==FOCUS_UNKNOWN)?0:ProductionDataTables()["SecondaryFocusMod"][m_secondary - 1][1]));
}

double ProdCenter::ResearchPoints() const
{
    return  m_workforce 
          *(  ((m_primary  ==FOCUS_UNKNOWN)?0:ProductionDataTables()["PrimaryFocusMod"][m_primary - 1][3])
            + ((m_secondary==FOCUS_UNKNOWN)?0:ProductionDataTables()["SecondaryFocusMod"][m_secondary - 1][3]));
}

double ProdCenter::PercentComplete() const
{
    int cost = ItemBuildCost();
    return (cost ? (m_rollover / cost) : 0);
}

double ProdCenter::ItemBuildCost() const
{
  return Cost(m_currently_building);
}

double ProdCenter::ProductionPoints() const
{
  return std::min(IndustryPoints(),AvailableMinerals());
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
   retval.AppendChild(XMLElement("m_max_workforce", lexical_cast<string>(m_max_workforce)));
   if (vis == UniverseObject::FULL_VISIBILITY) {
      retval.AppendChild(XMLElement("m_primary", lexical_cast<string>(m_primary)));
      retval.AppendChild(XMLElement("m_secondary", lexical_cast<string>(m_secondary)));
      retval.AppendChild(XMLElement("m_workforce", lexical_cast<string>(m_workforce)));
      retval.AppendChild(XMLElement("m_planet_type", lexical_cast<string>(m_planet_type)));
      retval.AppendChild(XMLElement("m_currently_building", lexical_cast<string>(m_currently_building)));
      retval.AppendChild(XMLElement("m_rollover", lexical_cast<string>(m_rollover)));
      retval.AppendChild(XMLElement("m_available_minerals", lexical_cast<string>(m_available_minerals)));
   }
   return retval;
}


void ProdCenter::SetPrimaryFocus(FocusType focus)
{
   m_primary = focus;
   m_prod_changed_sig();
}

void ProdCenter::SetSecondaryFocus(FocusType focus)
{
   m_secondary = focus;
   m_prod_changed_sig();
}

void ProdCenter::SetWorkforce(double workforce)
{
   m_workforce = workforce;
   m_prod_changed_sig();
}

void ProdCenter::SetMaxWorkforce(double max_workforce)
{
   m_max_workforce = max_workforce;
   m_prod_changed_sig();
}

void ProdCenter::SetPlanetType(PlanetType planet_type)
{
   m_planet_type = planet_type;
   m_prod_changed_sig();
}

void ProdCenter::SetProduction(ProdCenter::BuildType type)
{
   m_currently_building = type;
   m_prod_changed_sig();
}

void ProdCenter::MovementPhase( )
{
}

void ProdCenter::PopGrowthProductionResearchPhase( Empire *empire, const int system_id, const int planet_id )
{
  Universe* universe = &GetUniverse();
  
  // Update research:
  
  // look up planet
  UniverseObject* the_object = universe->Object( planet_id );  
  Planet* the_planet = dynamic_cast<Planet*> ( the_object );  
  
  if (the_planet->ResearchPoints() > 0)
  	empire->AddRP(the_planet->ResearchPoints());

  if (m_currently_building == DEF_BASE )
  {
    // for v0.2 we hard-code values for cost of bases
    int new_bases = UpdateBuildProgress( 200 );
    
    if ( new_bases > 0 )
    {
        // add base
        the_planet->AdjustDefBases( new_bases );

        // add sitrep
        SitRepEntry *p_entry = CreateBaseBuiltSitRep( system_id, planet_id );
        empire->AddSitRepEntry( p_entry );
     }
  }
  // V0.2 only - we would have a better way to know we're building different ships
  // for now enumerate through the ones we can build
  else if ( m_currently_building == ProdCenter::SCOUT )
  {
    UpdateShipBuildProgress( empire, system_id, planet_id, ShipDesign::SCOUT );
  }
  else if ( m_currently_building == ProdCenter::COLONY_SHIP )
  {
    UpdateShipBuildProgress( empire, system_id, planet_id, ShipDesign::COLONY );
  }
  else if ( m_currently_building == ProdCenter::MARKI )
  {
    UpdateShipBuildProgress( empire, system_id, planet_id, ShipDesign::MARK1 );
  }
  else if ( m_currently_building == ProdCenter::MARKII )
  {
    UpdateShipBuildProgress( empire, system_id, planet_id, ShipDesign::MARK2 );
  }
  else if ( m_currently_building == ProdCenter::MARKIII )
  {
    UpdateShipBuildProgress( empire, system_id, planet_id, ShipDesign::MARK3 );
  }
  else if ( m_currently_building == ProdCenter::MARKIV )
  {
    UpdateShipBuildProgress( empire, system_id, planet_id, ShipDesign::MARK4 );
  }
  
}


int ProdCenter::UpdateBuildProgress( int item_cost )
{
  double total_build_points =  m_rollover + ProductionPoints();// IndustryPoints();

  int new_items = total_build_points / item_cost;
    
  m_rollover = total_build_points - ( new_items * item_cost );

  return new_items;
}


void ProdCenter::UpdateShipBuildProgress(  Empire *empire, const int system_id, const int planet_id, ShipDesign::V02DesignID design_id )
{
    Universe* universe = &GetUniverse();
    ShipDesign ship_design;

    // get ship design we're trying to build
    if ( empire->CopyShipDesign( (int) design_id, ship_design ) )
    {
        int new_ships = UpdateBuildProgress( ship_design.cost );

        if ( new_ships > 0 )
        {
            UniverseObject* obj = universe->Object( system_id );
            System* the_system = dynamic_cast<System*> (obj);

            // create new fleet with new ship
            Fleet* new_fleet = new Fleet("", the_system->X(), the_system->Y(), empire->EmpireID() );
            int fleet_id = universe->Insert(new_fleet);
  
            // set name
            // TODO: What is the mechanism for determining new fleet name?
            std::string fleet_name( "New fleet " );
            fleet_name += boost::lexical_cast<std::string>( fleet_id );
            new_fleet->Rename( fleet_name );

            // insert fleet around this system
            the_system->Insert(new_fleet);
  
            // add new ship (s)
            for ( int i = 0; i < new_ships; i++ )
            {
                Ship *new_ship = new Ship(empire->EmpireID(), (int) design_id );
                int ship_id = universe->Insert( new_ship  );

                std::string ship_name( ship_design.name );
                ship_name += boost::lexical_cast<std::string>( ship_id );
                new_ship->Rename( ship_name );

                new_fleet->AddShip(ship_id);

                // add sitrep
                SitRepEntry *p_entry = CreateShipBuiltSitRep( ship_id, planet_id );
                empire->AddSitRepEntry( p_entry );
            }
        }
    }
}


