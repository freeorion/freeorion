#include "SitRepEntry.h"

#include "MultiplayerCommon.h"


const std::string SitRepEntry::SITREP_UPDATE_TAG = "SitRepUpdate";

SitRepEntry::SitRepEntry(const XMLElement& elem) : VarText( elem )
{
  if (elem.Tag() != "SitRepEntry")
     throw std::invalid_argument("Attempted to construct a SitRepEntry from an XMLElement that had a tag other than \"SitRepEntry\"");

  // set type
  m_type = (EntryType)(boost::lexical_cast<int>(elem.Attribute("EntryType" ) ) );
}

XMLElement SitRepEntry::XMLEncode() const
{
  XMLElement retval = VarText::XMLEncode( );

  retval.SetTag("SitRepEntry");

  retval.SetAttribute("EntryType", boost::lexical_cast<std::string>(m_type));

  return retval;
}

SitRepEntry *CreateTechResearchedSitRep( const std::string& tech_name )
{
  SitRepEntry  *pSitRep = new SitRepEntry( );

  pSitRep->SetType( SitRepEntry::TECH_RESEARCHED );

  XMLElement techID_elem( VarText::TECH_ID_TAG );
  techID_elem.SetAttribute("value", tech_name);
  pSitRep->GetVariables( ).AppendChild( techID_elem );

  return( pSitRep );
}
 
SitRepEntry *CreateBaseBuiltSitRep( int system_id, int planet_id )
{
  SitRepEntry  *pSitRep = new SitRepEntry( );

  pSitRep->SetType( SitRepEntry::BASE_BUILT );

  XMLElement system_elem( VarText::SYSTEM_ID_TAG );
  system_elem.SetAttribute("value",  boost::lexical_cast<std::string>( system_id ));
  pSitRep->GetVariables( ).AppendChild( system_elem );

  XMLElement planet_elem( VarText::PLANET_ID_TAG );
  planet_elem.SetAttribute("value",  boost::lexical_cast<std::string>( planet_id ));
  pSitRep->GetVariables( ).AppendChild( planet_elem );

  return( pSitRep );
}

SitRepEntry *CreateShipBuiltSitRep( int ship_id, int system_id )
{
  SitRepEntry  *pSitRep = new SitRepEntry( );

  pSitRep->SetType( SitRepEntry::SHIP_BUILT );

  XMLElement system_elem( VarText::SYSTEM_ID_TAG );
  system_elem.SetAttribute("value",  boost::lexical_cast<std::string>( system_id ));
  pSitRep->GetVariables( ).AppendChild( system_elem );

  XMLElement ship_elem( VarText::SHIP_ID_TAG );
  ship_elem.SetAttribute("value",  boost::lexical_cast<std::string>( ship_id ));
  pSitRep->GetVariables( ).AppendChild( ship_elem );

  return( pSitRep );
}

SitRepEntry *CreateBuildingBuiltSitRep(const std::string& building_name, int planet_id)
{
  SitRepEntry  *pSitRep = new SitRepEntry( );

  pSitRep->SetType( SitRepEntry::BUILDING_BUILT );

  XMLElement planet_elem( VarText::PLANET_ID_TAG );
  planet_elem.SetAttribute("value",  boost::lexical_cast<std::string>( planet_id ));
  pSitRep->GetVariables( ).AppendChild( planet_elem );

  XMLElement building_elem( VarText::BUILDING_ID_TAG );
  building_elem.SetAttribute("value",  building_name);
  pSitRep->GetVariables( ).AppendChild( building_elem );

  return( pSitRep );
}

SitRepEntry *CreateCombatSitRep(int empire_id, int victor_id, int system_id)
{
  SitRepEntry  *pSitRep = new SitRepEntry( );

  pSitRep->SetType(victor_id==-1? SitRepEntry::COMBAT_SYSTEM_NO_VICTOR
                                : (empire_id==victor_id?SitRepEntry::COMBAT_SYSTEM_WON : SitRepEntry::COMBAT_SYSTEM_LOST));

  XMLElement system_elem( VarText::SYSTEM_ID_TAG );
  system_elem.SetAttribute("value",  boost::lexical_cast<std::string>( system_id ));
  pSitRep->GetVariables( ).AppendChild( system_elem );

  return( pSitRep );
}

SitRepEntry *CreatePlanetStarvedToDeathSitRep(int system_id, int planet_id)
{
  SitRepEntry  *pSitRep = new SitRepEntry( );

  pSitRep->SetType(SitRepEntry::PLANET_LOST_STARVED_TO_DEATH);

  XMLElement planet_elem( VarText::PLANET_ID_TAG );
  planet_elem.SetAttribute("value",  boost::lexical_cast<std::string>( planet_id ));
  pSitRep->GetVariables( ).AppendChild( planet_elem );

  XMLElement system_elem( VarText::SYSTEM_ID_TAG );
  system_elem.SetAttribute("value",  boost::lexical_cast<std::string>( system_id ));
  pSitRep->GetVariables( ).AppendChild( system_elem );

  return( pSitRep );
}

SitRepEntry *CreatePlanetColonizedSitRep(int system_id, int planet_id) {
  SitRepEntry  *pSitRep = new SitRepEntry( );

  pSitRep->SetType(SitRepEntry::PLANET_COLONIZED);

  XMLElement planet_elem( VarText::PLANET_ID_TAG );
  planet_elem.SetAttribute("value",  boost::lexical_cast<std::string>( planet_id ));
  pSitRep->GetVariables( ).AppendChild( planet_elem );

  XMLElement system_elem( VarText::SYSTEM_ID_TAG );
  system_elem.SetAttribute("value",  boost::lexical_cast<std::string>( system_id ));
  pSitRep->GetVariables( ).AppendChild( system_elem );

   return( pSitRep );
 }

SitRepEntry *CreateFleetArrivedAtDestinationSitRep(int system_id, int fleet_id) {
  SitRepEntry  *pSitRep = new SitRepEntry( );

  pSitRep->SetType(SitRepEntry::FLEET_ARRIVED_AT_DESTINATION);

  XMLElement fleet_elem( VarText::FLEET_ID_TAG );
  fleet_elem.SetAttribute("value",  boost::lexical_cast<std::string>( fleet_id ));
  pSitRep->GetVariables( ).AppendChild( fleet_elem );

  XMLElement system_elem( VarText::SYSTEM_ID_TAG );
  system_elem.SetAttribute("value",  boost::lexical_cast<std::string>( system_id ));
  pSitRep->GetVariables( ).AppendChild( system_elem );

  return( pSitRep );
}