#include "PopCenter.h"

#include "../util/AppInterface.h"
#include "XMLDoc.h"

#include <boost/lexical_cast.hpp>

#include <stdexcept>

using boost::lexical_cast;


PopCenter::PopCenter() : 
   m_pop(0.0),
   m_max_pop(0.0),
   m_growth(0.0),
   m_env_growth_mod(1.0),
   m_race(-1),
   m_available_food(0.0)
{
}
   
PopCenter::PopCenter(double max_pop) : 
   m_pop(0.0),
   m_max_pop(max_pop),
   m_growth(0.0),
   m_env_growth_mod(1.0),
   m_race(-1),
   m_available_food(0.0)
{
}
   
PopCenter::PopCenter(double max_pop, int race) : 
   m_pop(0.0),
   m_max_pop(max_pop),
   m_growth(0.0),
   m_env_growth_mod(1.0),
   m_race(race),
   m_available_food(0.0)
{
}
   
PopCenter::PopCenter(const GG::XMLElement& elem)
{
    if (elem.Tag() != "PopCenter")
        throw std::invalid_argument("Attempted to construct a PopCenter from an XMLElement that had a tag other than \"PopCenter\"");

    try {
        m_pop = lexical_cast<double>(elem.Child("m_pop").Text());
        m_max_pop = lexical_cast<double>(elem.Child("m_max_pop").Text());
        m_growth = lexical_cast<double>(elem.Child("m_growth").Text());
        m_env_growth_mod = lexical_cast<double>(elem.Child("m_env_growth_mod").Text());
        m_race = lexical_cast<int>(elem.Child("m_race").Text());
        m_available_food = lexical_cast<double>(elem.Child("m_available_food").Text());
    } catch (const boost::bad_lexical_cast& e) {
        Logger().debugStream() << "Caught boost::bad_lexical_cast in PopCenter::PopCenter(); bad XMLElement was:";
        std::stringstream osstream;
        elem.WriteElement(osstream);
        Logger().debugStream() << "\n" << osstream.str();
        throw;
    }
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

GG::XMLElement PopCenter::XMLEncode(UniverseObject::Visibility vis) const
{
   // partial encode version.  PopCenter is always fully visible
   using GG::XMLElement;
   using boost::lexical_cast;

   XMLElement element("PopCenter");
   element.AppendChild(XMLElement("m_pop", lexical_cast<std::string>(m_pop)));
   element.AppendChild(XMLElement("m_max_pop", lexical_cast<std::string>(m_max_pop)));
   element.AppendChild(XMLElement("m_growth", lexical_cast<std::string>(m_growth)));
   element.AppendChild(XMLElement("m_env_growth_mod", lexical_cast<std::string>(m_env_growth_mod)));
   element.AppendChild(XMLElement("m_race", lexical_cast<std::string>(m_race)));
   element.AppendChild(XMLElement("m_available_food", lexical_cast<std::string>(m_available_food)));

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
    m_pop = std::min(AvailableFood(),std::min(m_pop + (m_pop * ((max_pop - m_pop) / max_pop) * m_env_growth_mod * 0.072), m_max_pop)); // 7.2% growth means pop doubles every 10 turns
}



