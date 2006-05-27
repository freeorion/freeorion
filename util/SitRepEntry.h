// -*- C++ -*-
#ifndef _SitRepEntry_h_
#define _SitRepEntry_h_

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
        BASE_BUILT,
        SHIP_BUILT,
        BUILDING_BUILT,
        TECH_RESEARCHED,
        COMBAT_SYSTEM_WON,
        COMBAT_SYSTEM_LOST,
        COMBAT_SYSTEM_NO_VICTOR,
        PLANET_LOST_STARVED_TO_DEATH,
		PLANET_COLONIZED,
		FLEET_ARRIVED_AT_DESTINATION,
        NUM_SITREP_TYPES
    };

    /** \name Structors */ //@{
    SitRepEntry() : m_type(INVALID_ENTRY_TYPE) {} ///< default ctor

    /** ctor that constructs a SitRepEntry object from an XMLElement. \throw std::invalid_argument May throw std::invalid_argument if \a elem does not encode a SitRepEntry object */
    SitRepEntry(const XMLElement& elem);
    //@}
   
    /** \name Accessors */ //@{
    /** encodes the SitRepEntry into an XML element */
    XMLElement XMLEncode() const;
    //@}

    void SetType( EntryType type ) { m_type = type; }
    EntryType GetType( ) { return m_type; }

private:
    EntryType                  m_type; ///< the type of SitRep this is

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};


// Sitrep constructors - for each SitRep type, there is a global constructor function See implementation file for
// examples

SitRepEntry *CreateTechResearchedSitRep(const std::string& tech_name);

SitRepEntry *CreateBaseBuiltSitRep(int system_id, int planet_id);

SitRepEntry *CreateShipBuiltSitRep(int ship_id, int system_id);

SitRepEntry *CreateBuildingBuiltSitRep(const std::string& building_name, int planet_id);

SitRepEntry *CreateCombatSitRep(int empire_id, int victor_id, int system_id);

SitRepEntry *CreatePlanetStarvedToDeathSitRep(int system_id, int planet_id);

SitRepEntry *CreatePlanetColonizedSitRep(int system_id, int planet_id);

SitRepEntry *CreateFleetArrivedAtDestinationSitRep(int system_id, int fleet_id);

// template implementations
template <class Archive>
void SitRepEntry::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(VarText)
        & BOOST_SERIALIZATION_NVP(m_type);
}

#endif // _SitRepEntry_h_
