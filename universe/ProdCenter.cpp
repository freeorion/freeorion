#include "ProdCenter.h"
#include "XMLDoc.h"

#include <boost/lexical_cast.hpp>
using boost::lexical_cast;

#include <stdexcept>

ProdCenter::ProdCenter() : 
   m_primary(BALANCED),
   m_secondary(BALANCED),
   m_workforce(0),
   m_industry_factor(0.0),
   m_currently_building(NOT_BUILDING)
{
}

ProdCenter::ProdCenter(double workforce) : 
   m_primary(BALANCED),
   m_secondary(BALANCED),
   m_workforce(workforce),
   m_industry_factor(0.0),
   m_currently_building(NOT_BUILDING)
{
}

ProdCenter::ProdCenter(const GG::XMLElement& elem) : 
   UniverseObject()
{
   if (elem.Tag() != "ProdCenter")
      throw std::invalid_argument("Attempted to construct a ProdCenter from an XMLElement that had a tag other than \"ProdCenter\"");
   // TODO
}

ProdCenter::~ProdCenter()
{
}

double ProdCenter::ProdPoints() const
{
	double retval = 0.0;
   // TODO
   return retval;
}

UniverseObject::Visibility ProdCenter::Visible(int empire_id) const
{
   // For a ProdCenter visibility will always be checked against
   // the implementing object, so this function will never be used.

   return FULL_VISIBILITY;
}


GG::XMLElement ProdCenter::XMLEncode() const
{
   using GG::XMLElement;
   using boost::lexical_cast;

   XMLElement element("ProdCenter.h");

   XMLElement visibility("visibility");
   visibility.SetAttribute( "value", lexical_cast<std::string>(FULL_VISIBILITY) );
   element.AppendChild(visibility);

   XMLElement primary("m_primary");
   primary.SetAttribute( "value", lexical_cast<std::string>(m_primary) );
   element.AppendChild(primary);

   XMLElement secondary("m_secondary");
   secondary.SetAttribute( "value", lexical_cast<std::string>(m_secondary) );
   element.AppendChild(secondary);

   XMLElement workforce("m_workforce");
   workforce.SetAttribute( "value", lexical_cast<std::string>(m_workforce) );
   element.AppendChild(workforce);

   XMLElement industry("m_industry_factor");
   industry.SetAttribute( "value", lexical_cast<std::string>(m_industry_factor) );
   element.AppendChild(industry);

   XMLElement currently_building("m_currently_building");
   currently_building.SetAttribute( "value", lexical_cast<std::string>(m_currently_building) );
   element.AppendChild(currently_building);

   XMLElement rollover("m_rollover");
   currently_building.SetAttribute( "value", lexical_cast<std::string>(m_rollover) );
   element.AppendChild(rollover);

   return element;

}

GG::XMLElement ProdCenter::XMLEncode(int empire_id) const
{
   // partial encode version.  Skips currently_building and rollover

   using GG::XMLElement;
   using boost::lexical_cast;

   XMLElement element("ProdCenter.h");

   XMLElement visibility("visibility");
   visibility.SetAttribute( "value", lexical_cast<std::string>(PARTIAL_VISIBILITY) );
   element.AppendChild(visibility);

   XMLElement primary("m_primary");
   primary.SetAttribute( "value", lexical_cast<std::string>(m_primary) );
   element.AppendChild(primary);

   XMLElement secondary("m_secondary");
   secondary.SetAttribute( "value", lexical_cast<std::string>(m_secondary) );
   element.AppendChild(secondary);

   XMLElement workforce("m_workforce");
   workforce.SetAttribute( "value", lexical_cast<std::string>(m_workforce) );
   element.AppendChild(workforce);

   XMLElement industry("m_industry_factor");
   industry.SetAttribute( "value", lexical_cast<std::string>(m_industry_factor) );
   element.AppendChild(industry);

   return element;

}


void ProdCenter::SetPrimaryFocus(FocusType focus)
{
   m_primary = focus;
}

void ProdCenter::SetSecondaryFocus(FocusType focus)
{
   m_secondary = focus;
}

void ProdCenter::AdjustWorkforce(double workforce)
{
   m_workforce += workforce;
}

void ProdCenter::BuildDefBase()
{
   // TODO
}

void ProdCenter::BuildShip(int id)
{
   // TODO
}

void ProdCenter::BuildIndustry()
{
   // TODO
}

void ProdCenter::DoResearch()
{
   // TODO
}

void ProdCenter::AdjustIndustry(double industry)
{
   m_industry_factor += industry;
}


void ProdCenter::MovementPhase(std::vector<SitRepEntry>& sit_reps)
{
   // TODO
}

void ProdCenter::PopGrowthProductionResearchPhase(std::vector<SitRepEntry>& sit_reps)
{
   // TODO
}

void ProdCenter::XMLMerge(const GG::XMLElement& elem)
{
   // TODO
}

