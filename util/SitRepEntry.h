#ifndef _SitRepEntry_h_
#define _SitRepEntry_h_


#ifndef _XMLDoc_h_
#include "../GG/XML/XMLDoc.h"
#endif

#include <string>
#include <vector>
#include <map>
   
   
#ifndef _XMLObjectFactory_h_
#include "../GG/XML/XMLObjectFactory.h"
#endif

enum BuildSitRepEntryType 
{
   MAX_INDUSTRY_HIT,
   MAX_TECH_HIT,
   SHIP_BUILT
};



struct CombatShipSummary 
{
   int     ShipTypeID;
   int     InitialShipCount;
   int     FinalShipCount;
};

typedef std::vector<CombatShipSummary> CombatShipList;
typedef std::map<int, CombatShipList> CombatFleetMap;  ///< maps empire ID's to CombatShipLists




/** Base class for SitRepEntries. Serves as base for ColonizeSitRepEntry, FleetArrivalSitRepEntry, CombatSitRepEntry, NewTechSitRepEntry, BuildSitRepEntry.
Provides unified interface for the UI to display the entries and for hyperlinking into the galaxy map.*/
class SitRepEntry
{
public:
   friend  class ClientApp;
   
   /** \name Structors */ //@{
   SitRepEntry();
   virtual ~SitRepEntry();
   //@}
   
   SitRepEntry(const GG::XMLElement& element);
   
   virtual int     ImageID() = 0;      ///< returns the ID of the image to display with this entry
   
   virtual GG::XMLElement XMLEncode() const =0;   ///< encodes the SitRepEntry into an XML element
   
   /// Adds generator functions to a given factory so that the factory can produce Generate instances of any SitRepEntry subclass
   static void InitObjectFactory( GG::XMLObjectFactory<SitRepEntry>& factory);
   
protected:
   virtual const std::string&  SummaryText() = 0;  ///< returns the string to display inthe SitRep
   virtual bool                ExecuteLink() = 0;  ///< causes the entry to trigger the appropriate UI display for this event, returns true on success
	
   std::string    m_summary_text;
};






/** class for SitRepEntries for build events */
class BuildSitRepEntry : public SitRepEntry
{
public:
   /** \name Structors */ //@{
   BuildSitRepEntry(BuildSitRepEntryType buildType, int planetID, int fleetID);
   BuildSitRepEntry(const GG::XMLElement& element);
   
   ~BuildSitRepEntry() {};
   //@}
   
   virtual GG::XMLElement XMLEncode() const; ///< encodes the SitRepEntry into an XML element
   
   virtual const std::string&  SummaryText();  ///< returns the string to display in the SitRep
   int                 ImageID();      ///< returns the ID of the image to display with this entry
   
protected:
   virtual bool    ExecuteLink();  ///< causes the entry to trigger the appropriate UI display for this event, returns true on success
   
   BuildSitRepEntryType   m_build_type;
   int                    m_planet_id;
   int                    m_fleet_id;
   
};




/** class for SitRepEntries for colonization attempts */
class ColonizeSitRepEntry : public SitRepEntry
{
public:
   /** \name Structors */ //@{
   ColonizeSitRepEntry(int planetID, int fleetID, bool colo_success, int failEmpireID);
   ColonizeSitRepEntry(const GG::XMLElement& element);
   
   ~ColonizeSitRepEntry() {};
   //@}
   
   virtual GG::XMLElement XMLEncode() const; ///< encodes the SitRepEntry into an XML element
   
   int     ImageID();         ///< returns the ID of the image to display with this entry
   int     PlanetID();        ///< returns ID of planet where colonization was attempted.  Provides accessibility to AI.
   int     FleetID();         ///< returns ID of the fleet.  Provides accessibility to AI.
   bool    ColonizeSuccess(); ///< returns whether or not the colonization was successful. Provides accessibility to AI.
   int     FailEmpireID();    ///< in the case that the attempt fails, returns the ID of the empire that was successful in colonizing the planet (currently the only possible cause for colonization failure is that a planet was already colonized).  Provides accessibility to AI.
   
protected:
   virtual const std::string&  SummaryText();     ///< returns the string to display in the SitRep
   virtual bool                ExecuteLink();  ///< causes the entry to trigger the appropriate UI display for this event, returns true on success.
   
   int            m_planet_id;
   int            m_fleet_id;
   bool           m_success;
   int            m_fail_empire_id;
};





/** class for SitRepEntries for battles */
class CombatSitRepEntry : public SitRepEntry
{
 public:
   /** \name Structors */ //@{
   CombatSitRepEntry(int systemID, int initialOwner, CombatFleetMap* fleetDataMap);
   virtual ~CombatSitRepEntry() {};
   //@}
   CombatSitRepEntry(const GG::XMLElement& elem);
   
   virtual GG::XMLElement XMLEncode() const; ///< encodes the SitRepEntry into an XML element
   
   
   
   int                    ImageID();         ///< returns the ID of the image to display with this entry
   int                    SystemID();        ///< returns ID of system where combat occurred.  Provides accessibility to AI.
   int                    InitialOwnerID();  ///< returns the ID of the empire that controlled the system prior to combat.  Provides accessbility to AI.
   const CombatFleetMap*  FleetDataMapPtr(); ///< returns a pointer to the combat results struct.  Provides accessbility to AI.
   
protected:
   virtual const std::string&     SummaryText();     ///< returns the string to display in the SitRep
   virtual bool                   ExecuteLink();  ///< causes the entry to trigger the appropriate UI display for this event, returns true on success.
   
   int            m_system_id;
   int            m_initial_owner;
   CombatFleetMap m_combat_fleet_map;
   
};



/** class for SitRepEntries for fleet arrivals */
class FleetArrivalSitRepEntry : public SitRepEntry
{
public:
   /** \name Structors */ //@{
   FleetArrivalSitRepEntry(int systemID, int fleetID, bool newExplore);
   ~FleetArrivalSitRepEntry() {};
   //@}
   FleetArrivalSitRepEntry(const GG::XMLElement& elem);
   
   virtual GG::XMLElement XMLEncode() const; ///< encodes the SitRepEntry into an XML element
   
   
   int     ImageID();         ///< returns the ID of the image to display with this entry
   int     SystemID();        ///< returns ID of system arrived at.  Provides accessibility to AI.
   int     FleetID();         ///< returns the ID of the fleet.  Provides accessibility to AI.
   bool    NewExplore();      ///< returns whether or not the system was explored for the first time.  Provides accessibility to AI.
   virtual const std::string&  SummaryText();  ///< returns the string to display in the SitRep
   
protected:
   virtual bool    ExecuteLink();     ///< causes the entry to trigger the appropriate UI display for this  event, returns true on success.
   int            m_system_id;
   int            m_fleet_id;
   bool           m_new_explore;
   
};



/** class for SitRepEntries for new tech advances */
class TechSitRepEntry : public SitRepEntry
{
public:
   /** \name Structors */ //@{
   TechSitRepEntry();
   ~TechSitRepEntry() {};
   //@}
   TechSitRepEntry(const GG::XMLElement& elem);
 
 
   virtual GG::XMLElement XMLEncode() const; ///< encodes the SitRepEntry into an XML element
  
   
   int                 ImageID();      ///< returns the ID of the image to display with this entry
  
protected:
   virtual bool                ExecuteLink();     ///< causes the entry to trigger the appropriate UI display for this event, returns true on success.    
   virtual const std::string&  SummaryText();  ///< returns the string to display in the SitRep
   
};







#endif // _SitRepEntry_h_
