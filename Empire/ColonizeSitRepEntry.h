#ifndef _ColonizeSitRepEntry_h_
#define _ColonizeSitRepEntry_h_

#ifndef _SitRepEntry_h_
#include "SitRepEntry.h"
#endif


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

#endif // _ColonizeSitRepEntry_h_

