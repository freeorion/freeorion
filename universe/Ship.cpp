#include "Ship.h"

#include "../util/AppInterface.h"
#include "Fleet.h"
#include "XMLDoc.h"

#include <log4cpp/Appender.hh>
#include <log4cpp/Category.hh>
#include <log4cpp/PatternLayout.hh>
#include <log4cpp/FileAppender.hh>
#include <boost/lexical_cast.hpp>
using boost::lexical_cast;
#include <stdexcept>

////////////////////////////////////////////////
// ShipDesign
////////////////////////////////////////////////
ShipDesign::ShipDesign() : 
   id(ShipDesign::SCOUT),
   empire(-1),
   name(""),
   attack(0),
   defense(0),
   cost(10000000),
   colonize( false )
{
   //TODO
}

ShipDesign::ShipDesign(const GG::XMLElement& elem)
{
   if (elem.Tag() != "ShipDesign" )
      throw std::invalid_argument("Attempted to construct a ShipDesign from an XMLElement that had a tag other than \"ShipDesign\"");

   id = lexical_cast<int> ( elem.Child("id").Attribute("value") );
   empire = lexical_cast<int> ( elem.Child("empire").Attribute("value") );
   name = elem.Child("name").Text();
   attack = lexical_cast<int> ( elem.Child("attack").Attribute("value") );
   defense = lexical_cast<int> ( elem.Child("defense").Attribute("value") );
   cost = lexical_cast<int> ( elem.Child("cost").Attribute("value") );

}

GG::XMLElement ShipDesign::XMLEncode() const
{
   using GG::XMLElement;
   using boost::lexical_cast;

   XMLElement element("ShipDesign");

   XMLElement sd_ID("id");
   sd_ID.SetAttribute( "value", lexical_cast<std::string>(id) );
   element.AppendChild(sd_ID);

   XMLElement sd_empire("empire");
   sd_empire.SetAttribute( "value", lexical_cast<std::string>(empire) );
   element.AppendChild(sd_empire);
   
   XMLElement sd_name("name");
   sd_name.SetText(name);
   element.AppendChild(sd_name);

   XMLElement sd_attack("attack");
   sd_attack.SetAttribute( "value", lexical_cast<std::string>(attack) );
   element.AppendChild(sd_attack);

   XMLElement sd_defense("defense");
   sd_defense.SetAttribute( "value", lexical_cast<std::string>(defense) );
   element.AppendChild(sd_defense);

   XMLElement sd_cost("cost");
   sd_cost.SetAttribute( "value", lexical_cast<std::string>(cost) );
   element.AppendChild(sd_cost);


   return element;

}


int ShipDesign::WarpSpeed() const
{
    return 1; // for 0.1, and the early revs.  This will change later
}

////////////////////////////////////////////////
// Ship
////////////////////////////////////////////////
Ship::Ship() : 
   m_design(ShipDesign())
{
   //TODO
}

Ship::Ship(int empire_id, int design_id)
{
   // This constructor should only be used by the server, will not work if called from client.
   
   // Lookup empire where design is located
    Empire* empire = Empires().Lookup(empire_id);


   if (empire->CopyShipDesign(design_id, m_design) != true)
   {
      throw std::invalid_argument("Attempted to construct a Ship with an invalid design ID");
   }


   
}

Ship::Ship(const GG::XMLElement& elem) : 
  UniverseObject(elem.Child("UniverseObject")),
  m_design(ShipDesign(elem.Child("ShipDesign")))
{
   if (elem.Tag().find( "Ship" ) == std::string::npos )
      throw std::invalid_argument("Attempted to construct a Ship from an XMLElement that had a tag other than \"Ship\"");

   m_fleet_id = lexical_cast<int> ( elem.Child("m_fleet_id").Attribute("value") );
}

Fleet* Ship::GetFleet() const
{
    return dynamic_cast<Fleet*>(GetUniverse().Object(m_fleet_id));
}

UniverseObject::Visibility Ship::Visible(int empire_id) const
{
   // Ship is visible if the fleet it is in is visible
    Empire* empire = Empires().Lookup(empire_id);
   if ((empire->HasFleet(FleetID())) || (empire->HasVisibleFleet(FleetID())))
   {
      return FULL_VISIBILITY;
   }

   return NO_VISIBILITY;
}


GG::XMLElement Ship::XMLEncode() const
{
   using GG::XMLElement;
   using boost::lexical_cast;
   using std::string;

   string ship_name( "Ship" );
   ship_name += boost::lexical_cast<std::string>( ID() );
   XMLElement element( ship_name );

   element.AppendChild( UniverseObject::XMLEncode() );

   element.AppendChild( m_design.XMLEncode() );

   XMLElement fleet_id("m_fleet_id");
   fleet_id.SetAttribute( "value", lexical_cast<std::string>(m_fleet_id) );
   element.AppendChild(fleet_id);

   return element;
}

GG::XMLElement Ship::XMLEncode(int empire_id) const
{
   // ships are always fully encoded so partial version is
   // the same as the full
   using GG::XMLElement;
   using boost::lexical_cast;
   using std::string;

   string ship_name( "Ship" );
   ship_name += boost::lexical_cast<std::string>( ID() );
   XMLElement element( ship_name );

   element.AppendChild( UniverseObject::XMLEncode() );

   element.AppendChild( m_design.XMLEncode() );

   return element;
}
  	
void Ship::MovementPhase( )
{
   //TODO
}

void Ship::PopGrowthProductionResearchPhase( )
{
   //TODO
}



