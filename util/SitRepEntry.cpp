#include "SitRepEntry.h"

#include "MultiplayerCommon.h"


namespace {
    bool temp_header_bool = RecordHeaderFile(SitRepEntryRevision());
    bool temp_source_bool = RecordSourceFile("$RCSfile$", "$Revision$");
}


const std::string SitRepEntry::SITREP_UPDATE_TAG = "SitRepUpdate";


SitRepEntry::SitRepEntry(const GG::XMLElement& elem) : VarText( elem )
{
  if (elem.Tag() != "SitRepEntry")
     throw std::invalid_argument("Attempted to construct a SitRepEntry from an XMLElement that had a tag other than \"SitRepEntry\"");

  // set type
  m_type = (EntryType)(boost::lexical_cast<int>(elem.Attribute("EntryType" ) ) );
}


GG::XMLElement SitRepEntry::XMLEncode() const
{
  GG::XMLElement retval = VarText::XMLEncode( );

  retval.SetTag("SitRepEntry");

  retval.SetAttribute("EntryType", boost::lexical_cast<std::string>(m_type));

  return retval;
}


SitRepEntry *CreateTechResearchedSitRep( const std::string& tech_name )
{
  SitRepEntry  *pSitRep = new SitRepEntry( );

  pSitRep->SetType( SitRepEntry::TECH_RESEARCHED );

  GG::XMLElement techID_elem( VarText::TECH_ID_TAG );
  techID_elem.SetAttribute("value", tech_name);
  pSitRep->GetVariables( ).AppendChild( techID_elem );

  return( pSitRep );
}
 

SitRepEntry *CreateMaxIndustrySitRep( const int system_id, const int planet_id )
{
  SitRepEntry  *pSitRep = new SitRepEntry( );

  pSitRep->SetType( SitRepEntry:: MAX_INDUSTRY_HIT );

  GG::XMLElement system_elem( VarText::SYSTEM_ID_TAG );
  system_elem.SetAttribute("value",  boost::lexical_cast<std::string>( system_id ));
  pSitRep->GetVariables( ).AppendChild( system_elem );

  GG::XMLElement planet_elem( VarText::PLANET_ID_TAG  );
  planet_elem.SetAttribute("value",  boost::lexical_cast<std::string>( planet_id ));
  pSitRep->GetVariables( ).AppendChild( planet_elem );

  return( pSitRep );
}

SitRepEntry *CreateBaseBuiltSitRep( const int system_id, const int planet_id )
{
  SitRepEntry  *pSitRep = new SitRepEntry( );

  pSitRep->SetType( SitRepEntry::BASE_BUILT );

  GG::XMLElement system_elem( VarText::SYSTEM_ID_TAG );
  system_elem.SetAttribute("value",  boost::lexical_cast<std::string>( system_id ));
  pSitRep->GetVariables( ).AppendChild( system_elem );

  GG::XMLElement planet_elem( VarText::PLANET_ID_TAG );
  planet_elem.SetAttribute("value",  boost::lexical_cast<std::string>( planet_id ));
  pSitRep->GetVariables( ).AppendChild( planet_elem );

  return( pSitRep );
}

SitRepEntry *CreateShipBuiltSitRep( const int ship_id, const int planet_id )
{
  SitRepEntry  *pSitRep = new SitRepEntry( );

  pSitRep->SetType( SitRepEntry::SHIP_BUILT );

  GG::XMLElement planet_elem( VarText::PLANET_ID_TAG );
  planet_elem.SetAttribute("value",  boost::lexical_cast<std::string>( planet_id ));
  pSitRep->GetVariables( ).AppendChild( planet_elem );

  GG::XMLElement ship_elem( VarText::SHIP_ID_TAG );
  ship_elem.SetAttribute("value",  boost::lexical_cast<std::string>( ship_id ));
  pSitRep->GetVariables( ).AppendChild( ship_elem );

  return( pSitRep );
}

SitRepEntry *CreateCombatSitRep(const int empire_id, const int victor_id, const int system_id)
{
  SitRepEntry  *pSitRep = new SitRepEntry( );

  pSitRep->SetType(victor_id==-1? SitRepEntry::COMBAT_SYSTEM_NO_VICTOR
                                : (empire_id==victor_id?SitRepEntry::COMBAT_SYSTEM_WON : SitRepEntry::COMBAT_SYSTEM_LOST));

  GG::XMLElement system_elem( VarText::SYSTEM_ID_TAG );
  system_elem.SetAttribute("value",  boost::lexical_cast<std::string>( system_id ));
  pSitRep->GetVariables( ).AppendChild( system_elem );

  return( pSitRep );
}

SitRepEntry *CreatePlanetStarvedToDeathSitRep(const int system_id, const int planet_id)
{
  SitRepEntry  *pSitRep = new SitRepEntry( );

  pSitRep->SetType(SitRepEntry::PLANET_LOST_STARVED_TO_DEATH);

  GG::XMLElement planet_elem( VarText::PLANET_ID_TAG );
  planet_elem.SetAttribute("value",  boost::lexical_cast<std::string>( planet_id ));
  pSitRep->GetVariables( ).AppendChild( planet_elem );

  GG::XMLElement system_elem( VarText::SYSTEM_ID_TAG );
  system_elem.SetAttribute("value",  boost::lexical_cast<std::string>( system_id ));
  pSitRep->GetVariables( ).AppendChild( system_elem );


  return( pSitRep );
}

