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
}

using boost::lexical_cast;

ProdCenter::ProdCenter() : 
   m_primary(BALANCED),
   m_secondary(BALANCED),
   m_workforce(0.0),
   m_max_workforce(0.0),
   m_planet_type(PT_TERRAN),
   m_currently_building(NOT_BUILDING),
   m_build_progress(0),
   m_rollover(0)
{
}

ProdCenter::ProdCenter(const GG::XMLElement& elem) : 
   m_primary(FOCUS_UNKNOWN),
   m_secondary(FOCUS_UNKNOWN),
   m_workforce(0.0),
   m_max_workforce(0.0),
   m_planet_type(PT_TERRAN),
   m_currently_building(BUILD_UNKNOWN),
   m_build_progress(0),
   m_rollover(0)
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
            m_planet_type = PlanetType(lexical_cast<int>(elem.Child("m_planet_type").Text()));
            m_currently_building = BuildType(lexical_cast<int>(elem.Child("m_currently_building").Text()));
            m_rollover = lexical_cast<double>(elem.Child("m_rollover").Text());
            m_build_progress = lexical_cast<double>(elem.Child("m_build_progress").Text());
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
    return m_workforce * (ProductionDataTables()["PrimaryFocusMod"][m_primary - 1][0] + 
                          ProductionDataTables()["SecondaryFocusMod"][m_secondary - 1][0]) 
    + ProductionDataTables()["EnviromentProductionMod"][m_planet_type][0];
}

double ProdCenter::IndustryPoints() const
{
    return m_workforce * (ProductionDataTables()["PrimaryFocusMod"][m_primary - 1][2] + 
                          ProductionDataTables()["SecondaryFocusMod"][m_secondary - 1][2]);
}

double ProdCenter::MiningPoints() const
{
    return m_workforce * (ProductionDataTables()["PrimaryFocusMod"][m_primary - 1][1] + 
                          ProductionDataTables()["SecondaryFocusMod"][m_secondary - 1][1]);
}

double ProdCenter::ResearchPoints() const
{
    return m_workforce * (ProductionDataTables()["PrimaryFocusMod"][m_primary - 1][3] + 
                          ProductionDataTables()["SecondaryFocusMod"][m_secondary - 1][3]);
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
      retval.AppendChild(XMLElement("m_build_progress", lexical_cast<string>(m_build_progress)));
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
}

void ProdCenter::SetProduction(ProdCenter::BuildType type)
{
   m_currently_building = type;
   m_prod_changed_sig();
}

void ProdCenter::MovementPhase( )
{
   // TODO
}

void ProdCenter::PopGrowthProductionResearchPhase( Empire *empire, const int system_id, const int planet_id )
{
  Universe* universe = &GetUniverse();

  if (CurrentlyBuilding() == DEF_BASE )
  {
    // for v0.1 we hard-code values for cost of bases
    int new_bases = UpdateBuildProgress( 200 );
    
    if ( new_bases > 0 )
    {
        // look up planet
        UniverseObject* the_object = universe->Object( planet_id );
        Planet* the_planet = dynamic_cast<Planet*> ( the_object );

        // add base
        the_planet->AdjustDefBases( new_bases );

        // add sitrep
        SitRepEntry *p_entry = CreateBaseBuiltSitRep( system_id, planet_id );
        empire->AddSitRepEntry( p_entry );

    }
  }

  // V0.2 only - we would have a better way to know we're building different ships
  // for now enumerate through the ones we can build
  else if ( CurrentlyBuilding() == ProdCenter::SCOUT )
  {
    UpdateShipBuildProgress( empire, system_id, planet_id, ShipDesign::SCOUT );
  }
  else if ( CurrentlyBuilding() == ProdCenter::COLONY_SHIP )
  {
    UpdateShipBuildProgress( empire, system_id, planet_id, ShipDesign::COLONY );
  }
  else if ( CurrentlyBuilding() == ProdCenter::MARKI )
  {
    UpdateShipBuildProgress( empire, system_id, planet_id, ShipDesign::MARK1 );
  }
  else if ( CurrentlyBuilding() == ProdCenter::MARKII )
  {
    UpdateShipBuildProgress( empire, system_id, planet_id, ShipDesign::MARK2 );
  }
  else if ( CurrentlyBuilding() == ProdCenter::MARKIII )
  {
    UpdateShipBuildProgress( empire, system_id, planet_id, ShipDesign::MARK3 );
  }
  else if ( CurrentlyBuilding() == ProdCenter::MARKIV )
  {
    UpdateShipBuildProgress( empire, system_id, planet_id, ShipDesign::MARK4 );
  }
  
}


int ProdCenter::UpdateBuildProgress( int item_cost )
{
  double new_build_progress =  BuildProgress() + Rollover() + IndustryPoints();

  int new_items = (int)new_build_progress / item_cost;
    
  if ( new_items > 0 )
  {
    // calculate rollover
    m_rollover = new_build_progress - ( new_items * item_cost );
    m_build_progress =  0.0;
  }
  else
  {
    // reset rollover
    m_rollover = 0.0;
    // adjust progress
    m_build_progress = new_build_progress;
  }

  return new_items;
}


void ProdCenter::UpdateShipBuildProgress(  Empire *empire, const int system_id, const int planet_id, ShipDesign::V01DesignID design_id )
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


