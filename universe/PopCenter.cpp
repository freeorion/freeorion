#include "PopCenter.h"
#include "XMLDoc.h"

#include <boost/lexical_cast.hpp>
using boost::lexical_cast;

#include <stdexcept>

PopCenter::PopCenter() : 
   m_pop(0.0),
   m_max_pop(0.0),
   m_growth(0.0),
   m_race(-1)
{
}
   
PopCenter::PopCenter(double max_pop) : 
   m_pop(0.0),
   m_max_pop(max_pop),
   m_growth(0.0),
   m_race(-1)
{
}
   
PopCenter::PopCenter(double max_pop, int race) : 
   m_pop(0.0),
   m_max_pop(max_pop),
   m_growth(0.0),
   m_race(race)
{
}
   
PopCenter::PopCenter(const GG::XMLElement& elem)
{

   if (elem.Tag() != "PopCenter")
      throw std::invalid_argument("Attempted to construct a PopCenter from an XMLElement that had a tag other than \"PopCenter\"");

   m_pop = lexical_cast<double> ( elem.Child("m_pop").Attribute("value") );
   m_max_pop = lexical_cast<double> ( elem.Child("m_max_pop").Attribute("value") );
   m_growth = lexical_cast<double> ( elem.Child("m_growth").Attribute("value") );
   m_race = lexical_cast<int> ( elem.Child("m_race").Attribute("value") );
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

UniverseObject::Visibility PopCenter::Visible(int empire_id) const
{
   // For a PopCenter visibility will always be checked against
   // the implementing object, so this function will never be used.

   return UniverseObject::FULL_VISIBILITY;
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

GG::XMLElement PopCenter::XMLEncode(int empire_id) const
{
   // partial encode version.  PopCenter is always fully encoded
   // so this function is identical to the full encode version

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


