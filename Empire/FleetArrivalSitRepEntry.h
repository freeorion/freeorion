#ifndef _FleetArrivalSitRepEntry_h_
#define _FleetArrivalSitRepEntry_h_

#ifndef _SitRepEntry_h_
#include "SitRepEntry.h"
#endif


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
#endif // _FleetArrivalSitRepEntry_h_


