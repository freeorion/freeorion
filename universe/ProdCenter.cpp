#include "ProdCenter.h"
#include "Planet.h"
#include "System.h"
#include "Fleet.h"
#include "XMLDoc.h"
#include "../Empire/Empire.h"
#include "../util/AppInterface.h"


#include <boost/lexical_cast.hpp>
using boost::lexical_cast;

#include <stdexcept>

ProdCenter::ProdCenter() : 
   m_primary(BALANCED),
   m_secondary(BALANCED),
   m_workforce(0.0),
   m_max_workforce(0.0),
   m_industry_factor(0.0),
   m_currently_building(NOT_BUILDING),
   m_build_progress(0),
   m_rollover(0)
{
}

ProdCenter::ProdCenter(const GG::XMLElement& elem) : 
   m_primary(BALANCED),
   m_secondary(BALANCED),
   m_workforce(0.0),
   m_max_workforce(0.0),
   m_industry_factor(0.0),
   m_currently_building(NOT_BUILDING),
   m_build_progress(0),
   m_rollover(0)
{

   if (elem.Tag() != "ProdCenter")
      throw std::invalid_argument("Attempted to construct a ProdCenter from an XMLElement that had a tag other than \"ProdCenter\"");

   UniverseObject::Visibility vis = UniverseObject::Visibility(lexical_cast<int> ( elem.Child("visibility").Attribute("value") ));

   m_primary = (FocusType) lexical_cast<int> ( elem.Child("m_primary").Attribute("value") );
   m_secondary = (FocusType) lexical_cast<int> ( elem.Child("m_secondary").Attribute("value") );
   m_workforce = lexical_cast<double> ( elem.Child("m_workforce").Attribute("value") );
   m_max_workforce = lexical_cast<double> ( elem.Child("m_max_workforce").Attribute("value") );
   m_industry_factor = lexical_cast<double> ( elem.Child("m_industry_factor").Attribute("value") );

   if (vis == UniverseObject::FULL_VISIBILITY)
   {
      m_currently_building = (BuildType) lexical_cast<int> ( elem.Child("m_currently_building").Attribute("value") );
      m_rollover = lexical_cast<double> ( elem.Child("m_rollover").Attribute("value") );
      m_build_progress = lexical_cast<double> ( elem.Child("m_build_progress").Attribute("value") );
   }
}

ProdCenter::~ProdCenter()
{
}

double ProdCenter::ProdPoints() const
{
    return m_workforce * 3.0 * (1.0 + IndustryFactor());
}

UniverseObject::Visibility ProdCenter::Visible(int empire_id) const
{
   // For a ProdCenter visibility will always be checked against
   // the implementing object, so this function will never be used.

   return UniverseObject::FULL_VISIBILITY;
}


GG::XMLElement ProdCenter::XMLEncode() const
{
   using GG::XMLElement;
   using boost::lexical_cast;
   using std::string;

   XMLElement element("ProdCenter");

   XMLElement temp("visibility");
   temp.SetAttribute("value", lexical_cast<string>(UniverseObject::FULL_VISIBILITY));
   element.AppendChild(temp);

   temp = XMLElement("m_primary");
   temp.SetAttribute("value", lexical_cast<string>(m_primary));
   element.AppendChild(temp);

   temp = XMLElement("m_secondary");
   temp.SetAttribute("value", lexical_cast<string>(m_secondary));
   element.AppendChild(temp);

   temp = XMLElement("m_workforce");
   temp.SetAttribute("value", lexical_cast<string>(m_workforce));
   element.AppendChild(temp);

   temp = XMLElement("m_max_workforce");
   temp.SetAttribute("value", lexical_cast<string>(m_max_workforce));
   element.AppendChild(temp);

   temp = XMLElement("m_industry_factor");
   temp.SetAttribute("value", lexical_cast<string>(m_industry_factor));
   element.AppendChild(temp);

   temp = XMLElement("m_currently_building");
   temp.SetAttribute("value", lexical_cast<string>(m_currently_building));
   element.AppendChild(temp);

   temp = XMLElement("m_rollover");
   temp.SetAttribute("value", lexical_cast<string>(m_rollover));
   element.AppendChild(temp);

   temp = XMLElement("m_build_progress");
   temp.SetAttribute("value", lexical_cast<string>(m_build_progress));
   element.AppendChild(temp);

   return element;

}

GG::XMLElement ProdCenter::XMLEncode(int empire_id) const
{
   // partial encode version. No rollover or currently building

   using GG::XMLElement;
   using boost::lexical_cast;

   XMLElement element("ProdCenter.h");

   XMLElement visibility("visibility");
   visibility.SetAttribute( "value", lexical_cast<std::string>(UniverseObject::PARTIAL_VISIBILITY) );
   element.AppendChild(visibility);

   XMLElement primary("m_primary");
   primary.SetAttribute( "value", lexical_cast<std::string>(m_primary) );
   element.AppendChild(primary);

   XMLElement secondary("m_secondary");
   secondary.SetAttribute( "value", lexical_cast<std::string>(m_secondary) );
   element.AppendChild(secondary);

   XMLElement workforce("m_workforce");
   workforce.SetAttribute( "value", lexical_cast<std::string>(m_workforce) );
   element.AppendChild(workforce);

   XMLElement max_workforce("m_max_workforce");
   max_workforce.SetAttribute( "value", lexical_cast<std::string>(m_max_workforce) );
   element.AppendChild(max_workforce);

   XMLElement industry("m_industry_factor");
   industry.SetAttribute( "value", lexical_cast<std::string>(m_industry_factor) );
   element.AppendChild(industry);


   return element;

}


void ProdCenter::SetPrimaryFocus(FocusType focus)
{
   m_primary = focus;
}

void ProdCenter::SetSecondaryFocus(FocusType focus)
{
   m_secondary = focus;
}

void ProdCenter::SetWorkforce(double workforce)
{
   m_workforce = workforce;
}

void ProdCenter::SetMaxWorkforce(double max_workforce)
{
   m_max_workforce = max_workforce;
}

void ProdCenter::SetProduction(ProdCenter::BuildType type)
{
    m_currently_building = type;
}


bool ProdCenter::AdjustIndustry(double industry)
{
   m_industry_factor += industry;

   if ( m_industry_factor >= ( m_workforce / m_max_workforce ) )
   {
     /// set hard max, send true for reaching max
     m_industry_factor = ( m_workforce / m_max_workforce );
     
     return true;
   }

   return false;
}


void ProdCenter::MovementPhase( )
{
   // TODO
}

void ProdCenter::PopGrowthProductionResearchPhase( Empire *empire, const int system_id, const int planet_id )
{
  Universe* universe = &GetUniverse();

  if (CurrentlyBuilding() == INDUSTRY_BUILD)
  {
    if ( AdjustIndustry(ProdPoints() / Workforce() * 0.01) )
    {
      /// display sitrep
      SitRepEntry *p_entry = CreateMaxIndustrySitRep( system_id, planet_id );
      
      empire->AddSitRepEntry( p_entry );
    }
  }
  else if (CurrentlyBuilding() == RESEARCH_BUILD )
  {      
    empire->AddRP( (int)( 3 * Workforce()  ) );
  }
  else if (CurrentlyBuilding() == DEF_BASE )
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

  // 0.1 only - we would have a better way to know we're building different ships
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
  double new_build_progress =  BuildProgress() + Rollover() + ProdPoints();

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

