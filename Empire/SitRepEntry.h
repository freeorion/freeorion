
#ifndef _FREEORION_SITREPENTRY_H_
#define _FREEORION_SITREPENTRY_H_

// this is a temporary file until  I get the real one from Joseph


#ifndef _XMLDoc_h_
#include "GG\XML\XMLDoc.h"
#endif

/**
* This is a preliminary version that is being used until Joseph
* sends Josh the real SitRepEntry headers
*
*/
class SitRepEntry
{
public:
    SitRepEntry(int EmpireID) { m_empire_id = EmpireID;};
    SitRepEntry(const GG::XMLElement& element) {};    
    
    int GetEmpireID() const { return m_empire_id; } ;

private:
    int m_empire_id;
    
};

// many derived classes. 
// See the IDS for some details.

#endif
