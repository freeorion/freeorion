#include "ColonizeSitRepEntry.h"

#define COLONIZE_SIT_REP_IMAGE  4  // to be replaced by global define..

ColonizeSitRepEntry::ColonizeSitRepEntry(int planetID, int fleetID, bool colo_success, int failEmpireID)
{
  m_planet_id = planetID;
  m_fleet_id = fleetID;
  m_success = colo_success;
  m_fail_empire_id = failEmpireID;
  printf("ColonizeSitRepEntry created!\n");
}


int ColonizeSitRepEntry::ImageID()
{
  return COLONIZE_SIT_REP_IMAGE;
}

int ColonizeSitRepEntry::PlanetID()
{
  return m_planet_id;
}

int ColonizeSitRepEntry::FleetID()
{
  return m_fleet_id;
}

bool ColonizeSitRepEntry::ColonizeSuccess()
{
  return m_success;
}

int ColonizeSitRepEntry::FailEmpireID()
{
  return m_fail_empire_id;
}

const std::string& ColonizeSitRepEntry::SummaryText()
{
  return m_summary_text;
}

bool ColonizeSitRepEntry::ExecuteLink()
{
  return true;
}
