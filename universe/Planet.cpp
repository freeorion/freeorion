#include "Planet.h"
#include "XMLDoc.h"
#include "Fleet.h"
#include "Ship.h"
#include "System.h"


#include <boost/lexical_cast.hpp>
using boost::lexical_cast;

#include "../util/AppInterface.h"
#include <stdexcept>

Planet::Planet() : 
   UniverseObject(),
   PopCenter(),
   ProdCenter()
{
}

Planet::Planet(PlanetType type, PlanetSize size) : 
   UniverseObject(),
   PopCenter(),
   ProdCenter()
{
   m_type = type;
   m_size = size;
   m_def_bases = 0;

   switch(size) {
   case SZ_TINY:
      SetMaxPop(10);
      break;
   case SZ_SMALL:
      SetMaxPop(30);
      break;
   case SZ_MEDIUM:
      SetMaxPop(50);
      break;
   case SZ_LARGE:
      SetMaxPop(70);
      break;
   case SZ_HUGE:
       SetMaxPop(90);
   }

}

Planet::Planet(const GG::XMLElement& elem) : 
   UniverseObject(elem.Child("UniverseObject")),
   PopCenter(elem.Child("PopCenter")),
   ProdCenter(elem.Child("ProdCenter"))
{
   using GG::XMLElement;

   if (elem.Tag() != "Planet")
      throw std::invalid_argument("Attempted to construct a Planet from an XMLElement that had a tag other than \"Planet\"");

   m_type = (PlanetType) lexical_cast<int> ( elem.Child("m_type").Attribute("value") );
   m_size = (PlanetSize) lexical_cast<int> ( elem.Child("m_size").Attribute("value") );
   m_def_bases = lexical_cast<int> ( elem.Child("m_def_bases").Attribute("value") );
}

UniverseObject::Visibility Planet::Visible(int empire_id) const
{
   // if system is visible, then planet is too. Full visibility
   // if owned by player, partial if not. 

   Empire* empire = (Empires()).Lookup(empire_id);

   if (empire->HasPlanet(ID()))
   {
      return FULL_VISIBILITY;
   }

   if (empire->HasExploredSystem(SystemID()))
   {
      return PARTIAL_VISIBILITY;
   }

   return NO_VISIBILITY;
}


GG::XMLElement Planet::XMLEncode() const
{
   using GG::XMLElement;
   using boost::lexical_cast;

   XMLElement element("Planet");

   element.AppendChild( UniverseObject::XMLEncode() );

   element.AppendChild( PopCenter::XMLEncode() );

   element.AppendChild( ProdCenter::XMLEncode() );

   XMLElement type("m_type");
   type.SetAttribute( "value", lexical_cast<std::string>(m_type) );
   element.AppendChild(type);

   XMLElement size("m_size");
   size.SetAttribute( "value", lexical_cast<std::string>(m_size) );
   element.AppendChild(size);

   XMLElement def_bases("m_def_bases");
   def_bases.SetAttribute( "value", lexical_cast<std::string>(m_def_bases) );
   element.AppendChild(def_bases);

   return element;
}

GG::XMLElement Planet::XMLEncode(int empire_id) const
{
   // Partial encoding of Planet for limited visibility

   using GG::XMLElement;
   using boost::lexical_cast;

   XMLElement element("Planet");

   // full encode of UniverseObject since owner list should be visible
   element.AppendChild( UniverseObject::XMLEncode() );

   // full encode for PopCenter, nothing is hidden there
   element.AppendChild( PopCenter::XMLEncode() );

   // partial encode of ProdCenter to hide the current build option info
   element.AppendChild( ProdCenter::XMLEncode(empire_id) );

   XMLElement type("m_type");
   type.SetAttribute( "value", lexical_cast<std::string>(m_type) );
   element.AppendChild(type);

   XMLElement size("m_size");
   size.SetAttribute( "value", lexical_cast<std::string>(m_size) );
   element.AppendChild(size);

   XMLElement def_bases("m_def_bases");
   def_bases.SetAttribute( "value", lexical_cast<std::string>(m_def_bases) );
   element.AppendChild(def_bases);

   return element;
}

void Planet::MovementPhase()
{
   // TODO
   StateChangedSignal()();
}

void Planet::PopGrowthProductionResearchPhase( )
{
  Empire* empire = (Empires()).Lookup( *Owners().begin() );

  /// do production center phase
  if (CurrentlyBuilding() == ProdCenter::INDUSTRY_BUILD)
  {
    if ( AdjustIndustry(ProdPoints() / Workforce() * 0.01) )
    {
      /// display sitrep
      SitRepEntry *p_entry = CreateMaxIndustrySitRep( SystemID(), ID() );
      
      empire->AddSitRepEntry( p_entry );
    }
  }
  else if (CurrentlyBuilding() == ProdCenter::RESEARCH_BUILD )
  {      
    empire->AddRP( (int)( 3 * Workforce()  ) );
  }
  else if (CurrentlyBuilding() == ProdCenter::DEF_BASE )
  {
    // for v0.1 we hard-code values for cost of bases
    int new_bases = UpdateBuildProgress( 200 );
    
    if ( new_bases > 0 )
    {
      // add base
      AdjustDefBases( new_bases );

      // add sitrep
      SitRepEntry *p_entry = CreateBaseBuiltSitRep( SystemID(), ID() );
      empire->AddSitRepEntry( p_entry );

    }
  }
  // 0.1 only - we would have a better way to know we're building different ships
  // for now enumerate through the ones we can build
  else if ( CurrentlyBuilding() == ProdCenter::SCOUT )
  {
    UpdateShipBuildProgress( ShipDesign::SCOUT );
  }
  else if ( CurrentlyBuilding() == ProdCenter::COLONY_SHIP )
  {
    UpdateShipBuildProgress( ShipDesign::COLONY );
  }

  PopCenter::PopGrowthProductionResearchPhase( );

  /// adjust workforce for prod center
  SetWorkforce(PopPoints());

  StateChangedSignal()();
}
 

void Planet::UpdateShipBuildProgress( ShipDesign::V01DesignID design_id )
{
  Empire* empire = (Empires()).Lookup( *Owners().begin() );
  Universe* universe = &GetUniverse();

  ShipDesign ship_design;

  // get ship design we're trying to build
  if ( empire->CopyShipDesign( (int) design_id, ship_design ) )
  {
    int new_ships = UpdateBuildProgress( ship_design.cost );

    if ( new_ships > 0 )
    {
      UniverseObject* obj = universe->Object( SystemID() );
      System* the_system = dynamic_cast<System*> (obj);

      // create new fleet with new ship
      Fleet* new_fleet = new Fleet("", the_system->X(), the_system->Y(), empire->EmpireID() );
      int fleet_id = universe->Insert(new_fleet);
	  
      // set name
      // TODO: What is the mechanism for determining new fleet name?
      std::string fleet_name( "Fleet" );
      fleet_name += boost::lexical_cast<std::string>( fleet_id );
      new_fleet->Rename( fleet_name );

      // insert fleet around this system
      the_system->Insert(new_fleet);
	  
      // add fleet to this empire
      empire->AddFleet(fleet_id);

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
	SitRepEntry *p_entry = CreateShipBuiltSitRep( ship_id, ID() );
        empire->AddSitRepEntry( p_entry );
      }
    }
  }
}
