#ifndef _CombatSitRepEntry_h_
#define _CombatSitRepEntry_h_

#ifndef _SitRepEntry_h_
#include "SitRepEntry.h"
#endif

#include <vector>
#include <map>

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

#endif // _CombatSitRepEntry_h_

