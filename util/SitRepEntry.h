// -*- C++ -*-
#ifndef _SitRepEntry_h_
#define _SitRepEntry_h_

#include "VarText.h"

#include <string>
#include <vector>

#include "Export.h"

/** Situation report entry, to be displayed in the SitRep screen. */
class FO_COMMON_API SitRepEntry : public VarText {
public:
    /** \name Structors */ //@{
    SitRepEntry();  ///< default ctor
    explicit SitRepEntry(const std::string& template_string, const std::string& icon = "");
    SitRepEntry(const std::string& template_string, int turn, const std::string& icon = "");
    //@}

    /** Accessors */ //@{
    int                 GetDataIDNumber(const std::string& tag) const;
    const std::string&  GetDataString(const std::string& tag) const;
    int                 GetTurn() const { return m_turn; }
    const std::string&  GetIcon() const { return m_icon; }
    std::string         Dump() const;
    //@}

private:
    int         m_turn;
    std::string m_icon;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Sitrep constructors for each SitRep type */
SitRepEntry CreateTechResearchedSitRep(const std::string& tech_name);
SitRepEntry CreateShipBuiltSitRep(int ship_id, int system_id, int shipdesign_id);
SitRepEntry CreateShipBlockBuiltSitRep(int system_id, int shipdesign_id, int number);
SitRepEntry CreateBuildingBuiltSitRep(int building_id, int planet_id);
FO_COMMON_API SitRepEntry CreateCombatSitRep(int system_id, int log_id);
FO_COMMON_API SitRepEntry CreateGroundCombatSitRep(int planet_id);
FO_COMMON_API SitRepEntry CreatePlanetCapturedSitRep(int planet_id, int empire_id);
FO_COMMON_API SitRepEntry CreateCombatDamagedObjectSitRep(int object_id, int combat_system_id, int empire_id);
FO_COMMON_API SitRepEntry CreateCombatDestroyedObjectSitRep(int object_id, int combat_system_id, int empire_id);
SitRepEntry CreatePlanetStarvedToDeathSitRep(int planet_id);
FO_COMMON_API SitRepEntry CreatePlanetColonizedSitRep(int planet_id);
FO_COMMON_API SitRepEntry CreateFleetArrivedAtDestinationSitRep(int system_id, int fleet_id, int recipient_empire_id);
SitRepEntry CreateEmpireEliminatedSitRep(int empire_id);
SitRepEntry CreateVictorySitRep(const std::string& reason_string, int empire_id);
SitRepEntry CreateSitRep(const std::string& template_string, const std::string& icon,
                         const std::vector<std::pair<std::string, std::string> >& parameters);

// template implementations
template <class Archive>
void SitRepEntry::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(VarText)
        & BOOST_SERIALIZATION_NVP(m_turn)
        & BOOST_SERIALIZATION_NVP(m_icon);
}

#endif // _SitRepEntry_h_
