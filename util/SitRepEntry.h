// -*- C++ -*-
#ifndef _SitRepEntry_h_
#define _SitRepEntry_h_

#ifndef _XMLDoc_h_
#include "XMLDoc.h"
#endif

#ifndef BOOST_LEXICAL_CAST_INCLUDED
#include <boost/lexical_cast.hpp>
#endif

#include <string>
#include <stdexcept>
#include <vector>


/** a simple SitRepEntry to be displayed in the SitRep screen. 
 *  This class describes a sitrep entry, for both the client and server. 
 *  Data for the SitRep is contained in a XML tree. The first element is
 *  always EntryType and represents the type of sitrep.
 *  Followed are elements representing the variables for the given SitRep.
 *  Their tag names correspond to placehold entries in the string used to
 *  display the SitRep.
 *  The server only maintains this XML data while the client decodes it and produces
 *  a string which is used to display the SitRep. This will contain tags for hyperlinks, etc
 *  For example:
 *  For SitRep type: MAX_INDUSTRY_HIT
 *  <SitRepEntry EntryType='0'>
 *     <m_planet value='123'/>
 *     <m_system value='56'/>
 *  </SitRepEntry>
 *
 *  The string for this entry would be:
 *  "The planet %m_planet% in the system %m_system% has reached it's maximum industry."
 */

#ifndef _VarText_h_
#include "VarText.h"
#endif

class SitRepEntry : public VarText
{
public:
    
    /** tag name of sitrep update */
    static const std::string SITREP_UPDATE_TAG;

    /** an enumeration of the types of entries 
        WARNING: make sure to update the LUT in UIClient.cpp which contain stringIDs for each type of SitRep
        This LUT is in Client because the server has no need for it - it's a UI issue. This design in a way breaks
        the data-hiding feature of a class, but is needed because both clients and server share this code. It's better
        to have the discrepancy here than to bloat the server with data it will not use */
    enum EntryType {
        INVALID_ENTRY_TYPE = -1,  ///< this is the EntryType for default-constructed SitRepEntrys; no others should have this type
        MAX_INDUSTRY_HIT,
        SHIP_BUILT,
        TECH_RESEARCHED,
        BASE_BUILT,
        COMBAT_SYSTEM_WON,
        COMBAT_SYSTEM_LOST,
        COMBAT_SYSTEM_NO_VICTOR,
        PLANET_LOST_STARVED_TO_DEATH,
        NUM_SITREP_TYPES
    };

    /** \name Structors */ //@{
    SitRepEntry() : m_type(INVALID_ENTRY_TYPE) {} ///< default ctor

    /** ctor that constructs a SitRepEntry object from an XMLElement. \throw std::invalid_argument May throw std::invalid_argument if \a elem does not encode a SitRepEntry object */
    SitRepEntry(const GG::XMLElement& elem);
    //@}
   
    /** \name Accessors */ //@{
    /** encodes the SitRepEntry into an XML element */
    GG::XMLElement XMLEncode() const;
    //@}

    void SetType( EntryType type ) { m_type = type; }
    EntryType GetType( ) { return m_type; }

private:

    EntryType                  m_type; ///< the type of SitRep this is
};


/*  Sitrep constructors - for each SitRep type, there is a global constructor function
 *  See implementation file for examples
 */

SitRepEntry *CreateMaxIndustrySitRep( const int system_id, const int planet_id );

SitRepEntry *CreateTechResearchedSitRep( const std::string& tech_name );

SitRepEntry *CreateBaseBuiltSitRep( const int system_id, const int planet_id );

SitRepEntry *CreateShipBuiltSitRep( const int ship_id, const int planet_id );

SitRepEntry *CreateCombatSitRep(const int empire_id, const int victor_id, const int system_id);

SitRepEntry *CreatePlanetStarvedToDeathSitRep(const int system_id, const int planet_id);

inline std::pair<std::string, std::string> SitRepEntryRevision()
{return std::pair<std::string, std::string>("$RCSfile$", "$Revision$");}

#endif // _SitRepEntry_h_
