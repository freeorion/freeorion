#include "Planet.h"
#include "../GG/XML/XMLDoc.h"

#include <boost/lexical_cast.hpp>
using boost::lexical_cast;

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
   m_system_id = -1;

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
      SetMaxPop(900);
   }

}

Planet::Planet(const GG::XMLElement& elem) : 
   UniverseObject(elem.Child("UniverseObject")),
   PopCenter(elem.Child("PopCenter")),
   ProdCenter(elem.Child("ProdCenter"))
{
   if (elem.Tag() != "Planet")
      throw std::invalid_argument("Attempted to construct a Planet from an XMLElement that had a tag other than \"Planet\"");
   // TODO
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

   XMLElement system_id("m_system_id");
   system_id.SetAttribute( "value", lexical_cast<std::string>(m_system_id) );
   element.AppendChild(system_id);

   XMLElement def_bases("m_def_bases");
   def_bases.SetAttribute( "value", lexical_cast<std::string>(m_def_bases) );
   element.AppendChild(def_bases);

   return element;
}

void Planet::MovementPhase(std::vector<SitRepEntry>& sit_reps)
{
   // TODO
}

void Planet::PopGrowthProductionResearchPhase(std::vector<SitRepEntry>& sit_reps)
{
   // TODO
}

void Planet::XMLMerge(const GG::XMLElement& elem)
{
   // TODO
}

