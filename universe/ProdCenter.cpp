#include "ProdCenter.h"
#include "XMLDoc.h"
#include "../Empire/Empire.h"

#include <boost/lexical_cast.hpp>
using boost::lexical_cast;

#include <stdexcept>

ProdCenter::ProdCenter() : 
   m_primary(BALANCED),
   m_secondary(BALANCED),
   m_workforce(0.0),
   m_max_workforce(0.0),
   m_industry_factor(0.0),
   m_currently_building(NOT_BUILDING),
   m_build_progress(0),
   m_rollover(0)
{
}

ProdCenter::ProdCenter(const GG::XMLElement& elem) : 
   m_primary(BALANCED),
   m_secondary(BALANCED),
   m_workforce(0.0),
   m_max_workforce(0.0),
   m_industry_factor(0.0),
   m_currently_building(NOT_BUILDING),
   m_build_progress(0),
   m_rollover(0)
{

   if (elem.Tag() != "ProdCenter")
      throw std::invalid_argument("Attempted to construct a ProdCenter from an XMLElement that had a tag other than \"ProdCenter\"");

   UniverseObject::Visibility vis = UniverseObject::Visibility(lexical_cast<int> ( elem.Child("visibility").Attribute("value") ));

   m_primary = (FocusType) lexical_cast<int> ( elem.Child("m_primary").Attribute("value") );
   m_secondary = (FocusType) lexical_cast<int> ( elem.Child("m_secondary").Attribute("value") );
   m_workforce = lexical_cast<double> ( elem.Child("m_workforce").Attribute("value") );
   m_max_workforce = lexical_cast<double> ( elem.Child("m_max_workforce").Attribute("value") );
   m_industry_factor = lexical_cast<double> ( elem.Child("m_industry_factor").Attribute("value") );

   if (vis == UniverseObject::FULL_VISIBILITY)
   {
      m_currently_building = (BuildType) lexical_cast<int> ( elem.Child("m_currently_building").Attribute("value") );
      m_rollover = lexical_cast<double> ( elem.Child("m_rollover").Attribute("value") );
      m_build_progress = lexical_cast<double> ( elem.Child("m_build_progress").Attribute("value") );
   }
}

ProdCenter::~ProdCenter()
{
}

double ProdCenter::ProdPoints() const
{
    return m_workforce * 3.0 * (1.0 + IndustryFactor());
}

UniverseObject::Visibility ProdCenter::Visible(int empire_id) const
{
   // For a ProdCenter visibility will always be checked against
   // the implementing object, so this function will never be used.

   return UniverseObject::FULL_VISIBILITY;
}


GG::XMLElement ProdCenter::XMLEncode() const
{
   using GG::XMLElement;
   using boost::lexical_cast;
   using std::string;

   XMLElement element("ProdCenter");

   XMLElement temp("visibility");
   temp.SetAttribute("value", lexical_cast<string>(UniverseObject::FULL_VISIBILITY));
   element.AppendChild(temp);

   temp = XMLElement("m_primary");
   temp.SetAttribute("value", lexical_cast<string>(m_primary));
   element.AppendChild(temp);

   temp = XMLElement("m_secondary");
   temp.SetAttribute("value", lexical_cast<string>(m_secondary));
   element.AppendChild(temp);

   temp = XMLElement("m_workforce");
   temp.SetAttribute("value", lexical_cast<string>(m_workforce));
   element.AppendChild(temp);

   temp = XMLElement("m_max_workforce");
   temp.SetAttribute("value", lexical_cast<string>(m_max_workforce));
   element.AppendChild(temp);

   temp = XMLElement("m_industry_factor");
   temp.SetAttribute("value", lexical_cast<string>(m_industry_factor));
   element.AppendChild(temp);

   temp = XMLElement("m_currently_building");
   temp.SetAttribute("value", lexical_cast<string>(m_currently_building));
   element.AppendChild(temp);

   temp = XMLElement("m_rollover");
   temp.SetAttribute("value", lexical_cast<string>(m_rollover));
   element.AppendChild(temp);

   temp = XMLElement("m_build_progress");
   temp.SetAttribute("value", lexical_cast<string>(m_build_progress));
   element.AppendChild(temp);

   return element;

}

GG::XMLElement ProdCenter::XMLEncode(int empire_id) const
{
   // partial encode version. No rollover or currently building

   using GG::XMLElement;
   using boost::lexical_cast;

   XMLElement element("ProdCenter.h");

   XMLElement visibility("visibility");
   visibility.SetAttribute( "value", lexical_cast<std::string>(UniverseObject::PARTIAL_VISIBILITY) );
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

   XMLElement max_workforce("m_max_workforce");
   max_workforce.SetAttribute( "value", lexical_cast<std::string>(m_max_workforce) );
   element.AppendChild(max_workforce);

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

void ProdCenter::SetWorkforce(double workforce)
{
   m_workforce = workforce;
}

void ProdCenter::SetMaxWorkforce(double max_workforce)
{
   m_max_workforce = max_workforce;
}

void ProdCenter::SetProduction(ProdCenter::BuildType type)
{
    m_currently_building = type;
}

void ProdCenter::SetRollover( double rollover )
{
  m_rollover = rollover;
}

void ProdCenter::SetBuildProgress( double build_progress )
{
  m_build_progress = build_progress;
}


bool ProdCenter::AdjustIndustry(double industry)
{
   m_industry_factor += industry;

   if ( m_industry_factor >= ( m_workforce / m_max_workforce ) )
   {
     /// set hard max, send true for reaching max
     m_industry_factor = ( m_workforce / m_max_workforce );
     
     return true;
   }

   return false;
}


void ProdCenter::MovementPhase( )
{
   // TODO
}

void ProdCenter::PopGrowthProductionResearchPhase( )
{

}


int ProdCenter::UpdateBuildProgress( int item_cost )
{
  double new_build_progress =  BuildProgress() + Rollover() + ProdPoints();

  int new_items = (int)new_build_progress / item_cost;
    
  if ( new_items > 0 )
  {
    // calculate rollover
    SetRollover( new_build_progress - ( new_items * item_cost ) );
    SetBuildProgress( 0.0 );
  }
  else
  {
    // reset rollover
    SetRollover( 0.0 );
    // adjust progress
    SetBuildProgress( new_build_progress );
  }

  return new_items;
}

