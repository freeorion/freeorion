#include "Planet.h"
#include "XMLDoc.h"

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

void Planet::MovementPhase(std::vector<SitRepEntry>& sit_reps)
{
   // TODO
   StateChangedSignal()();
}

void Planet::PopGrowthProductionResearchPhase(std::vector<SitRepEntry>& sit_reps)
{
    ProdCenter::PopGrowthProductionResearchPhase(sit_reps);
    PopCenter::PopGrowthProductionResearchPhase(sit_reps);
    AdjustWorkforce(PopPoints());
    StateChangedSignal()();
}



