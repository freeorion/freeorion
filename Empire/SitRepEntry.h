#ifndef _SitRepEntry_h_
#define _SitRepEntry_h_


#include <string>
#include <vector>
#include <map>


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


    virtual int     ImageID() = 0;      ///< returns the ID of the image to display with this entry

	  
 protected:
    virtual const std::string&  SummaryText() = 0;  ///< returns the string to display inthe SitRep
    virtual bool    ExecuteLink() = 0;  ///< causes the entry to trigger the appropriate UI display for this event, returns true on success

	std::string    m_summary_text;
};





enum BuildSitRepEntryType {
  MAX_INDUSTRY_HIT,
  MAX_TECH_HIT,
  SHIP_BUILT
};

/** class for SitRepEntries for build events */
class BuildSitRepEntry : public SitRepEntry
{
 public:
    /** \name Structors */ //@{
    BuildSitRepEntry(BuildSitRepEntryType buildType, int planetID, int fleetID);
	~BuildSitRepEntry() {};
	//@}

    const std::string&  SummaryText();  ///< returns the string to display in the SitRep
    int                 ImageID();      ///< returns the ID of the image to display with this entry

 protected:
    bool    ExecuteLink();  ///< causes the entry to trigger the appropriate UI display for this event, returns true on success

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
	~ColonizeSitRepEntry() {};
	//@}


    int                 ImageID();         ///< returns the ID of the image to display with this entry
	int                 PlanetID();        ///< returns ID of planet where colonization was attempted.  Provides accessibility to AI.
	int                 FleetID();         ///< returns ID of the fleet.  Provides accessibility to AI.
	bool                ColonizeSuccess(); ///< returns whether or not the colonization was successful. Provides accessibility to AI.
	int                 FailEmpireID();    ///< in the case that the attempt fails, returns the ID of the empire that was successful in colonizing the planet (currently the only possible cause for colonization failure is that a planet was already colonized).  Provides accessibility to AI.

 protected:
    const std::string&  SummaryText();     ///< returns the string to display in the SitRep
    bool           ExecuteLink();  ///< causes the entry to trigger the appropriate UI display for this event, returns true on success.
  
	int            m_planet_id;
	int            m_fleet_id;
	bool           m_success;
	int            m_fail_empire_id;
};


struct CombatShipSummary {
  int     ShipTypeID;
  int     InitialShipCount;
  int     FinalShipCount;
};

typedef std::vector<CombatShipSummary> CombatShipList;
typedef std::map<int, CombatShipList> CombatFleetMap;  ///< maps empire ID's to CombatShipLists



/** class for SitRepEntries for battles */
class CombatSitRepEntry : public SitRepEntry
{
 public:
    /** \name Structors */ //@{
    CombatSitRepEntry(int systemID, int initialOwner, CombatFleetMap* fleetDataMap);
	virtual ~CombatSitRepEntry() {};
	//@}

    int                    ImageID();         ///< returns the ID of the image to display with this entry
	int                    SystemID();        ///< returns ID of system where combat occurred.  Provides accessibility to AI.
	int                    InitialOwnerID();  ///< returns the ID of the empire that controlled the system prior to combat.  Provides accessbility to AI.
	const CombatFleetMap*  FleetDataMapPtr(); ///< returns a pointer to the combat results struct.  Provides accessbility to AI.
  
 protected:
    const std::string&     SummaryText();     ///< returns the string to display in the SitRep
    bool           ExecuteLink();  ///< causes the entry to trigger the appropriate UI display for this event, returns true on success.

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


    int     ImageID();         ///< returns the ID of the image to display with this entry
	int     SystemID();        ///< returns ID of system arrived at.  Provides accessibility to AI.
	int     FleetID();         ///< returns the ID of the fleet.  Provides accessibility to AI.
	bool    NewExplore();      ///< returns whether or not the system was explored for the first time.  Provides accessibility to AI.
    const std::string&  SummaryText();  ///< returns the string to display in the SitRep

 protected:
    bool    ExecuteLink();     ///< causes the entry to trigger the appropriate UI display for this  event, returns true on success.
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

    int                 ImageID();      ///< returns the ID of the image to display with this entry
    const std::string&  SummaryText();  ///< returns the string to display in the SitRep

 protected:
    bool                ExecuteLink();     ///< causes the entry to trigger the appropriate UI display for this event, returns true on success.    
  
};


#endif // _SitRepEntry_h_
