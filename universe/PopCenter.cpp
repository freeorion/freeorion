#include "PopCenter.h"
#include "../GG/XML/XMLDoc.h"

#include <boost/lexical_cast.hpp>
using boost::lexical_cast;

#include <stdexcept>

PopCenter::PopCenter() : 
   UniverseObject(),
   m_pop(0.0),
   m_max_pop(0.0),
   m_growth(0.0),
   m_race(-1)
{
}
   
PopCenter::PopCenter(double max_pop) : 
   UniverseObject(),
   m_pop(0.0),
   m_max_pop(max_pop),
   m_growth(0.0),
   m_race(-1)
{
}
   
PopCenter::PopCenter(double max_pop, int race) : 
   UniverseObject(),
   m_pop(0.0),
   m_max_pop(max_pop),
   m_growth(0.0),
   m_race(race)
{
}
   
PopCenter::PopCenter(const GG::XMLElement& elem) : 
   UniverseObject()
{
   if (elem.Tag() != "PopCenter")
      throw std::invalid_argument("Attempted to construct a PopCenter from an XMLElement that had a tag other than \"PopCenter\"");
   // TODO
}

PopCenter::~PopCenter()
{
}

double PopCenter::Inhabitants() const
{
	double retval = 0.0;
   // TODO
	return retval;
}

PopCenter::DensityType PopCenter::PopDensity() const
{
   DensityType retval = OUTPOST;
   // TODO
   return retval;
}

GG::XMLElement PopCenter::XMLEncode() const
{
   using GG::XMLElement;
   using boost::lexical_cast;

   XMLElement element("PopCenter");

   XMLElement pop("m_pop");
   pop.SetAttribute( "value", lexical_cast<std::string>(m_pop) );
   element.AppendChild(pop);

   XMLElement max_pop("m_max_pop");
   max_pop.SetAttribute( "value", lexical_cast<std::string>(m_max_pop) );
   element.AppendChild(max_pop);

   XMLElement growth("m_growth");
   growth.SetAttribute( "value", lexical_cast<std::string>(m_growth) );
   element.AppendChild(growth);

   XMLElement race("m_race");
   race.SetAttribute( "value", lexical_cast<std::string>(m_race) );
   element.AppendChild(race);

   return element;

}

double PopCenter::AdjustPop(double pop)
{
   double retval = 0.0;
   m_pop += pop;
   if (m_pop < 0.0) {
      retval = m_pop;
      m_pop = 0.0;
   } else if (m_max_pop < pop) {
      retval = m_pop - m_max_pop;
      m_pop = m_max_pop;
   }
   return retval;
}

void PopCenter::MovementPhase(std::vector<SitRepEntry>& sit_reps)
{
   // TODO
}

void PopCenter::PopGrowthProductionResearchPhase(std::vector<SitRepEntry>& sit_reps)
{
   // TODO
}

void PopCenter::XMLMerge(const GG::XMLElement& elem)
{
   // TODO
}

