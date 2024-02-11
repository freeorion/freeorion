#ifndef _SitRepEntry_h_
#define _SitRepEntry_h_

//! @file
//!     Declares the SitRepEntry calls and related factory functions.

#include "VarText.h"
#include "../universe/ConstantsFwd.h"

#include <string>
#include <vector>

#include "Export.h"

class ObjectMap;
class UniverseObject;
struct ScriptingContext;

//! Represents a situation report entry for a significant game event.
class FO_COMMON_API SitRepEntry final : public VarText {
public:
    SitRepEntry();

    SitRepEntry(const char* template_string, int turn, const char* icon,
                const char* label, bool stringtable_lookup);

    SitRepEntry(std::string&& template_string, int turn, std::string&& icon,
                std::string&& label, bool stringtable_lookup);

    [[nodiscard]] const std::string& GetDataString(const std::string& tag) const;
    [[nodiscard]] int                GetTurn() const noexcept        { return m_turn; }
    [[nodiscard]] const std::string& GetIcon() const noexcept        { return m_icon; }
    [[nodiscard]] const std::string& GetLabelString() const noexcept { return m_label; }
    [[nodiscard]] std::string        Dump() const;

private:
    int         m_turn = INVALID_GAME_TURN;
    std::string m_icon;
    std::string m_label;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

//! @name SitRepEntry factories
//!
//! Factory functions to create SitRepEntry(s) for specific situation report
//! events.
//!
//! @{
[[nodiscard]] SitRepEntry CreateTechResearchedSitRep(std::string tech_name, int current_turn);
[[nodiscard]] SitRepEntry CreateShipBuiltSitRep(int ship_id, int system_id, int shipdesign_id, int current_turn);
[[nodiscard]] SitRepEntry CreateShipBlockBuiltSitRep(int system_id, int shipdesign_id, int number, int current_turn);
[[nodiscard]] SitRepEntry CreateBuildingBuiltSitRep(int building_id, int planet_id, int current_turn);

[[nodiscard]] SitRepEntry CreateTechUnlockedSitRep(std::string tech_name, int current_turn);
[[nodiscard]] SitRepEntry CreatePolicyUnlockedSitRep(std::string policy_name, int current_turn);
[[nodiscard]] SitRepEntry CreateBuildingTypeUnlockedSitRep(std::string building_type_name, int current_turn);
[[nodiscard]] SitRepEntry CreateShipHullUnlockedSitRep(std::string ship_hull_name, int current_turn);
[[nodiscard]] SitRepEntry CreateShipPartUnlockedSitRep(std::string ship_part_name, int current_turn);

[[nodiscard]] FO_COMMON_API SitRepEntry CreateCombatSitRep(int system_id, int log_id, int empire_id, int current_turn);
[[nodiscard]] FO_COMMON_API SitRepEntry CreateGroundCombatSitRep(int planet_id, int empire_id, int current_turn);
[[nodiscard]] FO_COMMON_API SitRepEntry CreatePlanetCapturedSitRep(int planet_id, int empire_id, int current_turn);
[[nodiscard]] FO_COMMON_API SitRepEntry CreatePlanetRebelledSitRep(int planet_id, int empire_id, int current_turn);
[[nodiscard]] FO_COMMON_API SitRepEntry CreateCombatDamagedObjectSitRep(
    const UniverseObject* obj, int combat_system_id, int empire_id, int current_turn);
[[nodiscard]] FO_COMMON_API SitRepEntry CreateCombatDestroyedObjectSitRep(
    const UniverseObject* obj, int combat_system_id, int empire_id, int current_turn);
[[nodiscard]] SitRepEntry               CreatePlanetDepopulatedSitRep(int planet_id, int current_turn);
[[nodiscard]] FO_COMMON_API SitRepEntry CreatePlanetAnnexedSitRep(int planet_id, int original_owner_id, int annexer_empire_id, int current_turn);
[[nodiscard]] FO_COMMON_API SitRepEntry CreatePlanetColonizedSitRep(int planet_id, std::string species, int current_turn);
[[nodiscard]] FO_COMMON_API SitRepEntry CreatePlanetOutpostedSitRep(int planet_id, int current_turn);
[[nodiscard]] FO_COMMON_API SitRepEntry CreatePlanetEstablishFailedSitRep(int planet_id, int ship_id, int current_turn);
[[nodiscard]] FO_COMMON_API SitRepEntry CreatePlanetEstablishFailedVisibleOtherSitRep(int planet_id, int ship_id, int other_empire_id, int current_turn);
[[nodiscard]] FO_COMMON_API SitRepEntry CreatePlanetEstablishFailedArmedSitRep(int planet_id, int ship_id, int other_empire_id, int current_turn);

[[nodiscard]] FO_COMMON_API SitRepEntry CreatePlanetGiftedSitRep(int planet_id, int empire_id, int current_turn);
[[nodiscard]] FO_COMMON_API SitRepEntry CreateFleetGiftedSitRep(int fleet_id, int empire_id, int current_turn);

[[nodiscard]] FO_COMMON_API SitRepEntry CreateFleetArrivedAtDestinationSitRep(
    int system_id, int fleet_id, int recipient_empire_id, const ScriptingContext& context);
[[nodiscard]] FO_COMMON_API SitRepEntry CreateFleetBlockadedSitRep(
    int system_id, int blockaded_fleet_id, int blockaded_empire_id,
    int blockading_empire_id, const ScriptingContext& context);
[[nodiscard]] FO_COMMON_API SitRepEntry CreateFleetBlockadedSitRep(
    int system_id, int blockaded_fleet_id, int blockaded_empire_id, const ScriptingContext& context);

[[nodiscard]] SitRepEntry               CreateEmpireEliminatedSitRep(int empire_id, int current_turn);
[[nodiscard]] SitRepEntry               CreateVictorySitRep(std::string reason_string, int empire_id, int current_turn);
[[nodiscard]] FO_COMMON_API SitRepEntry CreateSitRep(std::string template_string, int turn,
                                                     std::string icon,
                                                     std::vector<std::pair<std::string, std::string>> parameters,
                                                     std::string label = "", bool stringtable_lookup = true);
//! @}

template <typename Archive>
void SitRepEntry::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(VarText)
        & BOOST_SERIALIZATION_NVP(m_turn)
        & BOOST_SERIALIZATION_NVP(m_icon)
        & BOOST_SERIALIZATION_NVP(m_label);
}


#endif
