#include "PopCenter.h"
#include "XMLDoc.h"

#include <boost/lexical_cast.hpp>

#include <stdexcept>

using boost::lexical_cast;


PopCenter::PopCenter() : 
   m_pop(0.0),
   m_max_pop(0.0),
   m_growth(0.0),
   m_env_growth_mod(1.0),
   m_race(-1)
{
}
   
PopCenter::PopCenter(double max_pop) : 
   m_pop(0.0),
   m_max_pop(max_pop),
   m_growth(0.0),
   m_env_growth_mod(1.0),
   m_race(-1)
{
}
   
PopCenter::PopCenter(double max_pop, int race) : 
   m_pop(0.0),
   m_max_pop(max_pop),
   m_growth(0.0),
   m_env_growth_mod(1.0),
   m_race(race)
{
}
   
PopCenter::PopCenter(const GG::XMLElement& elem)
{
   if (elem.Tag() != "PopCenter")
      throw std::invalid_argument("Attempted to construct a PopCenter from an XMLElement that had a tag other than \"PopCenter\"");

   m_pop = lexical_cast<double> ( elem.Child("m_pop").Text() );
   m_max_pop = lexical_cast<double> ( elem.Child("m_max_pop").Text() );
   m_growth = lexical_cast<double> ( elem.Child("m_growth").Text() );
   m_env_growth_mod = lexical_cast<double> ( elem.Child("m_env_growth_mod").Text() );
   m_race = lexical_cast<int> ( elem.Child("m_race").Text() );
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

#if 1 // how to do it...
   element.AppendChild(XMLElement("m_pop", lexical_cast<std::string>(m_pop)));
   element.AppendChild(XMLElement("m_max_pop", lexical_cast<std::string>(m_max_pop)));
   element.AppendChild(XMLElement("m_growth", lexical_cast<std::string>(m_growth)));
   element.AppendChild(XMLElement("m_env_growth_mod", lexical_cast<std::string>(m_env_growth_mod)));
   element.AppendChild(XMLElement("m_race", lexical_cast<std::string>(m_race)));
#else // and how not to do it...
   XMLElement pop("m_pop");
   pop.SetAttribute( "value", lexical_cast<std::string>(m_pop) );
   element.AppendChild(pop);

   XMLElement max_pop("m_max_pop");
   max_pop.SetAttribute( "value", lexical_cast<std::string>(m_max_pop) );
   element.AppendChild(max_pop);

   XMLElement growth("m_growth");
   growth.SetAttribute( "value", lexical_cast<std::string>(m_growth) );
   element.AppendChild(growth);

   XMLElement env_growth_mod("m_env_growth_mod");
   env_growth_mod.SetAttribute( "value", lexical_cast<std::string>(m_env_growth_mod) );
   element.AppendChild(env_growth_mod);

   XMLElement race("m_race");
   race.SetAttribute( "value", lexical_cast<std::string>(m_race) );
   element.AppendChild(race);
#endif

   return element;
}

GG::XMLElement PopCenter::XMLEncode(int empire_id) const
{
   // partial encode version.  PopCenter is always fully encoded
   // so this function is identical to the full encode version

   using GG::XMLElement;
   using boost::lexical_cast;

   XMLElement element("PopCenter");
   element.AppendChild(XMLElement("m_pop", lexical_cast<std::string>(m_pop)));
   element.AppendChild(XMLElement("m_max_pop", lexical_cast<std::string>(m_max_pop)));
   element.AppendChild(XMLElement("m_growth", lexical_cast<std::string>(m_growth)));
   element.AppendChild(XMLElement("m_env_growth_mod", lexical_cast<std::string>(m_env_growth_mod)));
   element.AppendChild(XMLElement("m_race", lexical_cast<std::string>(m_race)));
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

void PopCenter::MovementPhase( )
{
   // TODO
}

void PopCenter::PopGrowthProductionResearchPhase( )
{
    double max_pop = (m_max_pop == 0.0 ? 1.0 : m_max_pop);   // to prevent division by zero
    m_pop = std::min(m_pop + (m_pop * ((max_pop - m_pop) / max_pop) * m_env_growth_mod * 0.072), m_max_pop); // 7.2% growth means pop doubles every 10 turns
}


