#include "SitRepEntry.h"
#ifdef FREEORION_BUILD_HUMAN
#include "../client/human/HumanClientApp.h"
#include "../universe/ClientUniverse.h"
#include "../universe/Planet.h"
#include "../universe/Fleet.h"
#include "../universe/Ship.h"
#endif

#define TECH_SIT_REP_IMAGE  1  // to be replaced by global define..
#define BUILD_SIT_REP_IMAGE  2  // to be replaced by global define.
#define COLONIZE_SIT_REP_IMAGE  3  // to be replaced by global define..
#define COMBAT_SIT_REP_IMAGE  4  // to be replaced by global define..
#define FLEET_SIT_REP_IMAGE  5  // to be replaced by global define..

#include <boost/lexical_cast.hpp>
using boost::lexical_cast;

using GG::XMLElement;


///////////////////////////////////////////////////////////////////////
// Generator functions to construct subclass objects from XMLElements
///////////////////////////////////////////////////////////////////////

SitRepEntry* NewBuild(const XMLElement& elem) { return new BuildSitRepEntry(elem); };
SitRepEntry* NewCombat(const XMLElement& elem) { return new CombatSitRepEntry(elem); };
SitRepEntry* NewFleetArrival(const XMLElement& elem) { return new FleetArrivalSitRepEntry(elem); };
SitRepEntry* NewColonize(const XMLElement& elem) { return new ColonizeSitRepEntry(elem); };
SitRepEntry* NewTech(const XMLElement& elem) { return new TechSitRepEntry(elem); };

////////////////////////////////////////////////
// SitRepEntry
////////////////////////////////////////////////


SitRepEntry::SitRepEntry()
{
    
}

SitRepEntry::~SitRepEntry()
{
}

SitRepEntry::SitRepEntry(const GG::XMLElement& element)
{
    m_summary_text = element.Child("m_summary_text").Text();
}

void SitRepEntry::InitObjectFactory( GG::XMLObjectFactory<SitRepEntry>& factory)
{
    factory.AddGenerator("BuildSitRepEntry", &NewBuild);
    factory.AddGenerator("CombatSitRepEntry", &NewCombat);
    factory.AddGenerator("FleetArrivalSitRepEntry", &NewFleetArrival);
    factory.AddGenerator("ColonizeSitRepEntry", &NewColonize);
    factory.AddGenerator("TechSitRepEntry", &NewTech);

}

void SitRepEntry::EncodeBaseMembers(XMLElement& element) const
{
    XMLElement summary("m_summary_text", m_summary_text);
    element.AppendChild(summary);
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

BuildSitRepEntry::BuildSitRepEntry(const GG::XMLElement& elem) : SitRepEntry(elem)
{
    m_planet_id = lexical_cast<int>(elem.Child("m_planet_id").Attribute("value"));
    m_fleet_id = lexical_cast<int> (elem.Child("m_fleet_id").Attribute("value"));
    m_build_type = (BuildSitRepEntryType)lexical_cast<int> (elem.Child("m_build_type").Attribute("value"));
}

XMLElement BuildSitRepEntry::XMLEncode() const
{
    XMLElement element("BuildSitRepEntry");
    
    SitRepEntry::EncodeBaseMembers(element);
    
    XMLElement planetid("m_planet_id");
    planetid.SetAttribute("value", lexical_cast<std::string>(m_planet_id));
    element.AppendChild(planetid);
    
    XMLElement fleetid("m_fleet_id");
    fleetid.SetAttribute("value", lexical_cast<std::string>(m_fleet_id));
    element.AppendChild(fleetid);
    
    // cast is used in case lexical_cast
    // decides it doesn't want to cast an enum
    XMLElement buildtype("m_build_type");
    buildtype.SetAttribute("value", lexical_cast<std::string>((int)m_build_type));
    element.AppendChild(buildtype);
    
    return element;
}


int BuildSitRepEntry::ImageID()
{
   return BUILD_SIT_REP_IMAGE;
}

const std::string& BuildSitRepEntry::SummaryText()
{
#ifdef FREEORION_BUILD_HUMAN
   if (m_summary_text.empty())
   {
      HumanClientApp* client_app = HumanClientApp::GetApp();
      ClientUniverse universe = client_app->Universe();
      const UniverseObject* uni_obj;
      const Planet* planet;
      const Fleet* fleet;
      const Ship* ship;
      Fleet::const_iterator fleet_citr;

      switch(m_build_type) {
     
      case MAX_INDUSTRY_HIT:
         uni_obj = universe.Object(m_planet_id);
         planet = dynamic_cast<const Planet*>(uni_obj);
         if (!planet)
         {
            client_app->Logger().errorStream() << "BuildSitRepEntry::SummaryText : Planet with ID " << m_planet_id << " is NULL or of the wrong type in universe object list!";
            m_summary_text = "An error has occurred.  Invalid event.";
            return m_summary_text;
         }         
         m_summary_text = "The maximum industry level for the current population has been hit on " + planet->Name();
         break;

      case MAX_TECH_HIT:
         m_summary_text = "Your empire has now reached the maximum technology level.  Nothing can be gained by further research.";
         break;

      case SHIP_BUILT:  
         uni_obj = universe.Object(m_planet_id);
         planet = dynamic_cast<const Planet*>(uni_obj);
         if (!planet)
         {
            client_app->Logger().errorStream() << "BuildSitRepEntry::SummaryText : Planet with ID " << m_planet_id << " is NULL or of the wrong type in universe object list!";
            m_summary_text = "An error has occurred.  Invalid event.";
            return m_summary_text;
         }

         uni_obj = universe.Object(m_fleet_id);
         fleet = dynamic_cast<const Fleet*>(uni_obj);
         if (!fleet)
         {
            client_app->Logger().errorStream() << "BuildSitRepEntry::SummaryText : Fleet with ID " << m_fleet_id << " is NULL or of the wrong type in universe object list!";
            m_summary_text = "An error has occurred.  Invalid event.";
            return m_summary_text;
         }
         fleet_citr = fleet->begin();

         uni_obj = universe.Object(*fleet_citr);
         ship = dynamic_cast<const Ship*>(uni_obj);
         if (!ship)
         {
            client_app->Logger().errorStream() << "BuildSitRepEntry::SummaryText : Ship with ID " << *fleet_citr << " is NULL or of the wrong type in universe object list!";
            m_summary_text = "An error has occurred.  Invalid event.";
            return m_summary_text;
         }

         if (1 == 1) // TODO: add fleet size accessor in Fleet.h
         {
            m_summary_text = "A new " + ship->Name() + " has been built on " + planet->Name() + " and is awaiting orders.";
         }
         else
         {
            m_summary_text = "2 new " + ship->Name() + "'s have been build on " + planet->Name() + " and are awaiting orders.";
         }
         break;

      default:
         client_app->Logger().errorStream() << "BuildSitRepEntry::SummaryText : Invalid build event type: " << m_build_type;
         m_summary_text = "Unknown build event type!";       
      }
   }
#endif
   return m_summary_text;
}

bool BuildSitRepEntry::ExecuteLink()
{
#ifdef FREEORION_BUILD_HUMAN
   HumanClientApp* client_app = HumanClientApp::GetApp();
   ClientUniverse universe = client_app->Universe();
   const UniverseObject* uni_obj;
   const Planet* planet;
   const Fleet* fleet;   
   
   // TODO: implement ClientUI in HumanClientApp
   //   ClientUI* client_ui; // = xxxx
   
   switch(m_build_type) {
   case MAX_INDUSTRY_HIT:
      uni_obj = universe.Object(m_planet_id);
      planet = dynamic_cast<const Planet*>(uni_obj);
      if (!planet)
      {
         client_app->Logger().errorStream() << "BuildSitRepEntry::ExecuteLink : Planet with ID " << m_planet_id << " is NULL or of the wrong type in universe object list!";
         m_summary_text = "An error has occurred.  Invalid event.";
         return false;
      }

      /*
        if (client_ui.ZoomTo(*Planet) == false)
        {
        client_app->Logger().errorStream() << "BuildSitRepEntry::ExecuteLink : Could not zoom to planet with ID " << m_planet_id;
        return false;
        } 
      */         
      break;

   case MAX_TECH_HIT:
      // there is no link for the event. 
      // TODO: add LinkExists() to all SitRepEntries
      client_app->Logger().errorStream() << "BuildSitRepEntry::ExecuteLink : Attempting to link for MAX TECH entry for which no link exists.";
      return false;
      break;

   case SHIP_BUILT:
      // zoom to fleet
      uni_obj = universe.Object(m_fleet_id);
      fleet = dynamic_cast<const Fleet*>(uni_obj);
      if (!fleet)
      {
         client_app->Logger().errorStream() << "BuildSitRepEntry::ExecuteText : Fleet with ID " << m_fleet_id << " is NULL or of the wrong type in universe object list!";
         m_summary_text = "An error has occurred.  Invalid event.";
         return false;
      }
      /*
        if (client_ui.ZoomTo(*Fleet) == false)
        {
        client_app->Logger().errorStream() << "BuildSitRepEntry::ExecuteLink : Could not zoom to fleet with ID " << m_fleet_id;
        return false;
        } 
      */
      break;

   default:
      client_app->Logger().errorStream() << "BuildSitRepEntry::ExecuteLink : Invalid build event type: " << m_build_type;
      return false;
   }   
#endif
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

ColonizeSitRepEntry::ColonizeSitRepEntry(const XMLElement& elem) : SitRepEntry(elem)
{
    // write me!
}

XMLElement ColonizeSitRepEntry::XMLEncode() const
{   
    XMLElement elem("ColonizeSitRepEntry");
    // write me!
    return elem;
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
#ifdef FREEORION_BUILD_HUMAN
   if (m_summary_text.empty())
   {
      HumanClientApp* client_app = HumanClientApp::GetApp();

      // get fleet name
      // get planet name

      if (m_success)
      {
         // build string
      }
      else
      {
         // build string
      }
   }
#endif
   return m_summary_text;
}

bool ColonizeSitRepEntry::ExecuteLink()
{
#ifdef FREEORION_BUILD_HUMAN
   HumanClientApp* client_app = HumanClientApp::GetApp();

   // zoom to planet
#endif
   return true;
}


////////////////////////////////////////////////
// CombatSitRepEntry
////////////////////////////////////////////////


CombatSitRepEntry::CombatSitRepEntry(int systemID, int initialOwner, CombatFleetMap* fleetDataMap)
{
   m_system_id = systemID;
   m_initial_owner = initialOwner;
   memcpy((void*)&m_combat_fleet_map, (void*)fleetDataMap, sizeof(CombatFleetMap));   
}

CombatSitRepEntry::CombatSitRepEntry(const XMLElement& elem) : SitRepEntry(elem)
{
    // write me!
}

XMLElement CombatSitRepEntry::XMLEncode() const
{   
    XMLElement elem("CombatSitRepEntry");
    // write me!
    return elem;
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
#ifdef FREEORION_BUILD_HUMAN
   if (m_summary_text.empty())
   {
      HumanClientApp* client_app = HumanClientApp::GetApp();
      std::string victor_name;
      // get planet name
      // start causualty_string
      // for empires in combat fleet map
      //    get name
      //    list lost x of y _ship_type
      //    if any ships remain, store ID in victor_name
      // build string
   }   
#endif
   return m_summary_text;
}

const CombatFleetMap* CombatSitRepEntry::FleetDataMapPtr()
{
   return &m_combat_fleet_map;
}

bool CombatSitRepEntry::ExecuteLink()
{
#ifdef FREEORION_BUILD_HUMAN
   HumanClientApp* client_app = HumanClientApp::GetApp();
   // zoom to system
#endif
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
FleetArrivalSitRepEntry::FleetArrivalSitRepEntry(const XMLElement& elem) : SitRepEntry(elem)
{
    // write me!
}

XMLElement FleetArrivalSitRepEntry::XMLEncode() const
{   
    XMLElement elem("FleetArrivalSitRepEntry");
    // write me!
    return elem;
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
#ifdef FREEORION_BUILD_HUMAN
   if (m_summary_text.empty())
   {
      HumanClientApp* client_app = HumanClientApp::GetApp();
      // get fleet name
      // get system name
      // build string
   }
#endif
   return m_summary_text;
}

bool FleetArrivalSitRepEntry::ExecuteLink()
{
#ifdef FREEORION_BUILD_HUMAN
   // zoom to fleet
#endif
   return true;
}


////////////////////////////////////////////////
// TechSitRepEntry
////////////////////////////////////////////////

TechSitRepEntry::TechSitRepEntry()
{
   // TODO: constructor to take tech ID
}

TechSitRepEntry::TechSitRepEntry(const XMLElement& elem) : SitRepEntry(elem)
{
    // write me!
}

XMLElement TechSitRepEntry::XMLEncode() const
{   
    XMLElement elem("TechSitRepEntry");
    // write me!
    return elem;
}

int TechSitRepEntry::ImageID()
{
   return TECH_SIT_REP_IMAGE;
}

const std::string& TechSitRepEntry::SummaryText()
{
   if (m_summary_text.empty())
   {
      // build string
   }
   return m_summary_text;
}

bool TechSitRepEntry::ExecuteLink()
{
   // zoom to tech info
   return true;
}
