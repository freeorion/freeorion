#include "Ship.h"
#include "../GG/XML/XMLDoc.h"

#include <boost/lexical_cast.hpp>
using boost::lexical_cast;
#include <stdexcept>

////////////////////////////////////////////////
// ShipDesign
////////////////////////////////////////////////
ShipDesign::ShipDesign() : 
   race(-1),
   name(""),
   attack(0),
   defense(0),
   cost(10000000)
{
   //TODO
}

ShipDesign::ShipDesign(const GG::XMLElement& elem)
{
   if (elem.Tag() != "ShipDesign")
      throw std::invalid_argument("Attempted to construct a ShipDesign from an XMLElement that had a tag other than \"ShipDesign\"");
   //TODO
}

GG::XMLElement ShipDesign::XMLEncode() const
{


}


////////////////////////////////////////////////
// Ship
////////////////////////////////////////////////
Ship::Ship() : 
   m_design(ShipDesign())
{
   //TODO
}

Ship::Ship(int race, int design_id)
{
   //TODO
}

Ship::Ship(const GG::XMLElement& elem) : 
   UniverseObject(elem.Child("UniverseObject"))
{
   if (elem.Tag() != "Ship")
      throw std::invalid_argument("Attempted to construct a Ship from an XMLElement that had a tag other than \"Ship\"");
   //TODO
}

GG::XMLElement Ship::XMLEncode() const
{
   using GG::XMLElement;
   using boost::lexical_cast;

   XMLElement element("Ship");

   element.AppendChild( UniverseObject::XMLEncode() );

   element.AppendChild( m_design.XMLEncode() );

   return element;
}
  	
void Ship::MovementPhase(std::vector<SitRepEntry>& sit_reps)
{
   //TODO
}

void Ship::PopGrowthProductionResearchPhase(std::vector<SitRepEntry>& sit_reps)
{
   //TODO
}

void Ship::XMLMerge(const GG::XMLElement& elem)
{
   //TODO
}

