#include "FleetArrivalSitRepEntry.h"

#define FLEET_SIT_REP_IMAGE  3  // to be replaced by global define..

FleetArrivalSitRepEntry::FleetArrivalSitRepEntry(int systemID, int fleetID, bool newExplore)
{
  m_system_id = systemID;
  m_fleet_id = fleetID;
  m_new_explore = newExplore;
  printf("FleetArrivalSitRepEntry created!\n");
}


int FleetArrivalSitRepEntry::ImageID()
{
  return FLEET_SIT_REP_IMAGE;
}

int FleetArrivalSitRepEntry::SystemID()
{
  return m_system_id;
}

int FleetArrivalSitRepEntry::FleetID()
{
  return m_fleet_id;
}

bool FleetArrivalSitRepEntry::NewExplore()
{
  return m_new_explore;
}

const std::string& FleetArrivalSitRepEntry::SummaryText()
{
  return m_summary_text;
}

bool FleetArrivalSitRepEntry::ExecuteLink()
{
  return true;
}
