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
   colonize( false ),
   description("")
{
   //TODO
}

ShipDesign::ShipDesign(const GG::XMLElement& elem)
{
   if (elem.Tag() != "ShipDesign" )
      throw std::invalid_argument("Attempted to construct a ShipDesign from an XMLElement that had a tag other than \"ShipDesign\"");

   id = lexical_cast<int>(elem.Child("id").Text());
   empire = lexical_cast<int>(elem.Child("empire").Text());
   name = elem.Child("name").Text();
   attack = lexical_cast<int>(elem.Child("attack").Text());
   defense = lexical_cast<int>(elem.Child("defense").Text());
   cost = lexical_cast<int>(elem.Child("cost").Text());
   colonize = lexical_cast<bool>(elem.Child("colonize").Text());
   description = elem.Child("description").Text();
}

GG::XMLElement ShipDesign::XMLEncode() const
{
   using GG::XMLElement;
   using boost::lexical_cast;

   XMLElement retval("ShipDesign");
   retval.AppendChild(XMLElement("id", lexical_cast<std::string>(id)));
   retval.AppendChild(XMLElement("empire", lexical_cast<std::string>(empire)));
   retval.AppendChild(XMLElement("name", name));
   retval.AppendChild(XMLElement("attack", lexical_cast<std::string>(attack)));
   retval.AppendChild(XMLElement("defense", lexical_cast<std::string>(defense)));
   retval.AppendChild(XMLElement("cost", lexical_cast<std::string>(cost)));
   retval.AppendChild(XMLElement("colonize", lexical_cast<std::string>(colonize)));
   retval.AppendChild(XMLElement("description", description));
   return retval;

}


int ShipDesign::WarpSpeed() const
{
    return 1; // for 0.2, and the early revs.  This will change later
}

////////////////////////////////////////////////
// Ship
////////////////////////////////////////////////
Ship::Ship() : 
   m_design(ShipDesign())
{
}

Ship::Ship(int empire_id, int design_id)
{
   // Lookup empire where design is located
   Empire* empire = Empires().Lookup(empire_id);
   if (!empire->CopyShipDesign(design_id, m_design))
      throw std::invalid_argument("Attempted to construct a Ship with an invalid design ID");

   AddOwner(empire_id);
}

Ship::Ship(const GG::XMLElement& elem) : 
  UniverseObject(elem.Child("UniverseObject")),
  m_design(ShipDesign(elem.Child("ShipDesign")))
{
    if (elem.Tag().find( "Ship" ) == std::string::npos )
        throw std::invalid_argument("Attempted to construct a Ship from an XMLElement that had a tag other than \"Ship\"");

    try {
        m_fleet_id = lexical_cast<int> ( elem.Child("m_fleet_id").Text() );
    } catch (const boost::bad_lexical_cast& e) {
        Logger().debugStream() << "Caught boost::bad_lexical_cast in Ship::Ship(); bad XMLElement was:";
        std::stringstream osstream;
        elem.WriteElement(osstream);
        Logger().debugStream() << "\n" << osstream.str();
        throw;
    }
}

Fleet* Ship::GetFleet() const
{
    return dynamic_cast<Fleet*>(GetUniverse().Object(m_fleet_id));
}

UniverseObject::Visibility Ship::GetVisibility(int empire_id) const
{
  UniverseObject::Visibility vis = NO_VISIBILITY;

  if (empire_id == Universe::ALL_EMPIRES || OwnedBy(empire_id))
      vis = FULL_VISIBILITY;
  else
      vis = PARTIAL_VISIBILITY; // TODO: do something smarter here, such as a range check vs. owned systems and fleets

  // Ship is visible if its fleet is visible
  return FleetID() == INVALID_OBJECT_ID ? NO_VISIBILITY : (GetFleet()?GetFleet()->GetVisibility(empire_id):vis);
}

bool Ship::IsArmed() const
{
    return (m_design.attack > 0);
}

GG::XMLElement Ship::XMLEncode(int empire_id/* = Universe::ALL_EMPIRES*/) const
{
   using GG::XMLElement;
   using boost::lexical_cast;
   using std::string;
   XMLElement retval("Ship" + boost::lexical_cast<std::string>(ID()));
   retval.AppendChild(UniverseObject::XMLEncode(empire_id));
   retval.AppendChild(m_design.XMLEncode());
   retval.AppendChild(XMLElement("m_fleet_id", lexical_cast<std::string>(m_fleet_id)));
   return retval;
}
  	
void Ship::MovementPhase( )
{
   //TODO
}

void Ship::PopGrowthProductionResearchPhase( )
{
   //TODO
}
