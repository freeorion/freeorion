#include "BuildSitRepEntry.h"

#define BUILD_SIT_REP_IMAGE  2  // to be replaced by global define..

BuildSitRepEntry::BuildSitRepEntry(BuildSitRepEntryType buildType, int planetID, int fleetID)
{
  m_planet_id = planetID;
  m_fleet_id = fleetID;
  m_build_type = buildType;
  printf("BuildSitRepEntry created!\n");
}


int BuildSitRepEntry::ImageID()
{
  return BUILD_SIT_REP_IMAGE;
}

const std::string& BuildSitRepEntry::SummaryText()
{
  return m_summary_text;
}

bool BuildSitRepEntry::ExecuteLink()
{
  return true;
}
