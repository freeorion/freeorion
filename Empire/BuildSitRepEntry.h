#ifndef _BuildSitRepEntry_h_
#define _BuildSitRepEntry_h_

#ifndef _SitRepEntry_h_
#include "SitRepEntry.h"
#endif

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

#endif // _BuildSitRepEntry_h_

