#include "CombatSitRepEntry.h"

#define COMBAT_SIT_REP_IMAGE  1  // to be replaced by global define..

CombatSitRepEntry::CombatSitRepEntry(int systemID, int initialOwner, CombatFleetMap* fleetDataMap)
{
  m_system_id = systemID;
  m_initial_owner = initialOwner;
  printf("CombatSitRepEntry created!\n");
}


int CombatSitRepEntry::ImageID()
{
  return COMBAT_SIT_REP_IMAGE;
}

int CombatSitRepEntry::SystemID()
{
  return m_system_id;
}

int CombatSitRepEntry::InitialOwnerID()
{
  return m_initial_owner;
}

const std::string& CombatSitRepEntry::SummaryText()
{
  return m_summary_text;
}

const CombatFleetMap* CombatSitRepEntry::FleetDataMapPtr()
{
  return &m_combat_fleet_map;
}

bool CombatSitRepEntry::ExecuteLink()
{
  return true;
}
