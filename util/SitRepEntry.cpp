#include "SitRepEntry.h"


#define TECH_SIT_REP_IMAGE  1  // to be replaced by global define..
#define BUILD_SIT_REP_IMAGE  2  // to be replaced by global define.
#define COLONIZE_SIT_REP_IMAGE  3  // to be replaced by global define..
#define COMBAT_SIT_REP_IMAGE  4  // to be replaced by global define..
#define FLEET_SIT_REP_IMAGE  5  // to be replaced by global define..




SitRepEntry::SitRepEntry()
{
}

SitRepEntry::~SitRepEntry()
{
}




////////////////////////////////////////////////
// BuildSitRepEntry
////////////////////////////////////////////////

BuildSitRepEntry::BuildSitRepEntry(BuildSitRepEntryType buildType, int planetID, int fleetID)
{
   m_planet_id = planetID;
   m_fleet_id = fleetID;
   m_build_type = buildType;
   
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



////////////////////////////////////////////////
// ColonizeSitRepEntry
////////////////////////////////////////////////


ColonizeSitRepEntry::ColonizeSitRepEntry(int planetID, int fleetID, bool colo_success, int failEmpireID)
{
   m_planet_id = planetID;
   m_fleet_id = fleetID;
   m_success = colo_success;
   m_fail_empire_id = failEmpireID;
   
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


////////////////////////////////////////////////
// CombatSitRepEntry
////////////////////////////////////////////////


CombatSitRepEntry::CombatSitRepEntry(int systemID, int initialOwner, CombatFleetMap* fleetDataMap)
{
   m_system_id = systemID;
   m_initial_owner = initialOwner;
   
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


////////////////////////////////////////////////
// FleetArrivalSitRepEntry
////////////////////////////////////////////////


FleetArrivalSitRepEntry::FleetArrivalSitRepEntry(int systemID, int fleetID, bool newExplore)
{
   m_system_id = systemID;
   m_fleet_id = fleetID;
   m_new_explore = newExplore;
   
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


////////////////////////////////////////////////
// TechSitRepEntry
////////////////////////////////////////////////

TechSitRepEntry::TechSitRepEntry()
{

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
