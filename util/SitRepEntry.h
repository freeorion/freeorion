// -*- C++ -*-
#ifndef _SitRepEntry_h_
#define _SitRepEntry_h_

#ifndef BOOST_LEXICAL_CAST_INCLUDED
#include <boost/lexical_cast.hpp>
#endif

#include <string>
#include <stdexcept>
#include <vector>


/** a simple SitRepEntry to be displayed in the SitRep screen. This class describes a sitrep entry, for both the client
    and server.
 */

#ifndef _VarText_h_
#include "VarText.h"
#endif

class SitRepEntry : public VarText
{
public:
    /** tag name of sitrep update */
    static const std::string SITREP_UPDATE_TAG;

    /** An enumeration of the types of sitrep entries.
      * SitRepEntry::SitRepTemplateString needs to be updated for any new
      * EntryType that is added. */
    enum EntryType {
        INVALID_ENTRY_TYPE = -1,  ///< this is the EntryType for default-constructed SitRepEntrys; no others should have this type
        SHIP_BUILT,
        BUILDING_BUILT,
        TECH_RESEARCHED,
        COMBAT_SYSTEM,
        PLANET_CAPTURED,
        PLANET_LOST_STARVED_TO_DEATH,
        PLANET_COLONIZED,
        FLEET_ARRIVED_AT_DESTINATION,
        EMPIRE_ELIMINATED,
        VICTORY,
        NUM_SITREP_TYPES
    };
    static const std::string&   SitRepTemplateString(EntryType entry_type);

    /** \name Structors */ //@{
    SitRepEntry();  ///< default ctor
    SitRepEntry(EntryType entry_type);
    //@}

    /** \name Accessors */ //@{
    EntryType           GetType() const { return m_type; }
    const std::string&  TemplateString() const;
    //@}

private:
    EntryType       m_type; ///< the type of SitRep this is

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Sitrep constructors for each SitRep type */
SitRepEntry* CreateTechResearchedSitRep(const std::string& tech_name);
SitRepEntry* CreateShipBuiltSitRep(int ship_id, int system_id);
SitRepEntry* CreateBuildingBuiltSitRep(const std::string& building_name, int planet_id);
SitRepEntry* CreateCombatSitRep(int system_id);
SitRepEntry* CreatePlanetCapturedSitRep(int planet_id, const std::string& empire_name);
SitRepEntry* CreatePlanetStarvedToDeathSitRep(int planet_id);
SitRepEntry* CreatePlanetColonizedSitRep(int planet_id);
SitRepEntry* CreateFleetArrivedAtDestinationSitRep(int system_id, int fleet_id);
SitRepEntry* CreateEmpireEliminatedSitRep(const std::string& empire_name);
SitRepEntry* CreateVictorySitRep(const std::string& reason_string, const std::string& empire_name);

// template implementations
template <class Archive>
void SitRepEntry::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(VarText)
        & BOOST_SERIALIZATION_NVP(m_type);
}

#endif // _SitRepEntry_h_
