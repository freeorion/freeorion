#ifndef _TechSitRepEntry_h_
#define _TechSitRepEntry_h_

#ifndef _SitRepEntry_h_
#include "SitRepEntry.h"
#endif


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

#endif // _TechSitRepEntry_h_

