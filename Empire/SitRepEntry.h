#ifndef _SitRepEntry_h_
#define _SitRepEntry_h_


#include <string>


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

#endif // _SitRepEntry_h_
