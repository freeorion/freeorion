#include "TechSitRepEntry.h"

#define TECH_SIT_REP_IMAGE  2  // to be replaced by global define..

TechSitRepEntry::TechSitRepEntry()
{
  printf("TechSitRepEntry created!\n");
}


int TechSitRepEntry::ImageID()
{
  return TECH_SIT_REP_IMAGE;
}

const std::string& TechSitRepEntry::SummaryText()
{
  return m_summary_text;
}

bool TechSitRepEntry::ExecuteLink()
{
  return true;
}
