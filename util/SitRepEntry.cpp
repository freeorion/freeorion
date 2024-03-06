#include "SitRepEntry.h"

#include "i18n.h"
#include "Logger.h"
#include "../universe/Building.h"
#include "../universe/Planet.h"
#include "../universe/System.h"
#include "../universe/Ship.h"
#include "../universe/Fleet.h"
#include "../universe/Universe.h"

namespace {
#if defined(__cpp_lib_constexpr_string) && ((!defined(__GNUC__) || (__GNUC__ > 12) || (__GNUC__ == 12 && __GNUC_MINOR__ >= 2))) && ((!defined(_MSC_VER) || (_MSC_VER >= 1934))) && ((!defined(__clang_major__) || (__clang_major__ >= 17)))
    constexpr std::string EMPTY_STRING;
#else
    const std::string EMPTY_STRING;
#endif
}


SitRepEntry::SitRepEntry() :
    m_icon("/icons/sitrep/generic.png")
{}

SitRepEntry::SitRepEntry(const char* template_string, int turn, const char* icon,
                         const char* label, bool stringtable_lookup) :
    SitRepEntry(std::string(template_string), turn, std::string(icon),
                std::string(label), stringtable_lookup)
{}

SitRepEntry::SitRepEntry(std::string&& template_string, int turn,
                         std::string&& icon, std::string&& label,
                         bool stringtable_lookup) :
    VarText(std::move(template_string), stringtable_lookup),
    m_turn(turn),
    m_icon(icon.empty() ? "/icons/sitrep/generic.png" : std::move(icon)),
    m_label(std::move(label))
{}

const std::string& SitRepEntry::GetDataString(const std::string& tag) const {
    const auto elem = m_variables.find(tag);
    if (elem == m_variables.end())
        return EMPTY_STRING;
    return elem->second;
}

std::string SitRepEntry::Dump() const {
    std::string retval = "SitRep template_string = \"" + m_template_string + "\"";
    for (const auto& variable : m_variables)
        retval += " " + variable.first + " = " + variable.second;
    retval += " turn = " + std::to_string(m_turn);
    retval += " icon = " + m_icon;
    retval += " label = " + m_label;
    return retval;
}

SitRepEntry CreateTechResearchedSitRep(std::string tech_name, int current_turn) {
    SitRepEntry sitrep(
        UserStringNop("SITREP_TECH_RESEARCHED"),
        current_turn,
        "icons/sitrep/tech_researched.png",
        UserStringNop("SITREP_TECH_RESEARCHED_LABEL"), true);
    sitrep.AddVariable(VarText::TECH_TAG, std::move(tech_name));
    return sitrep;
}

SitRepEntry CreateShipBuiltSitRep(int ship_id, int system_id, int shipdesign_id, int current_turn) {
    SitRepEntry sitrep(
        UserStringNop("SITREP_SHIP_BUILT"),
        current_turn + 1,
        "icons/sitrep/ship_produced.png",
        UserStringNop("SITREP_SHIP_BUILT_LABEL"), true);
    sitrep.AddVariable(VarText::SYSTEM_ID_TAG, std::to_string(system_id));
    sitrep.AddVariable(VarText::SHIP_ID_TAG,   std::to_string(ship_id));
    sitrep.AddVariable(VarText::DESIGN_ID_TAG, std::to_string(shipdesign_id));
    return sitrep;
}

SitRepEntry CreateShipBlockBuiltSitRep(int system_id, int shipdesign_id, int number, int current_turn) {
    SitRepEntry sitrep(
        UserStringNop("SITREP_SHIP_BATCH_BUILT"),
        current_turn + 1,
        "icons/sitrep/ship_produced.png",
        UserStringNop("SITREP_SHIP_BATCH_BUILT_LABEL"), true);
    sitrep.AddVariable(VarText::SYSTEM_ID_TAG, std::to_string(system_id));
    sitrep.AddVariable(VarText::DESIGN_ID_TAG, std::to_string(shipdesign_id));
    sitrep.AddVariable(VarText::RAW_TEXT_TAG,  std::to_string(number));
    return sitrep;
}

SitRepEntry CreateBuildingBuiltSitRep(int building_id, int planet_id, int current_turn) {
    SitRepEntry sitrep(
        UserStringNop("SITREP_BUILDING_BUILT"),
        current_turn + 1,
        "icons/sitrep/building_produced.png",
        UserStringNop("SITREP_BUILDING_BUILT_LABEL"), true);
    sitrep.AddVariable(VarText::PLANET_ID_TAG,   std::to_string(planet_id));
    sitrep.AddVariable(VarText::BUILDING_ID_TAG, std::to_string(building_id));
    return sitrep;
}

SitRepEntry CreateTechUnlockedSitRep(std::string tech_name, int current_turn) {
    SitRepEntry sitrep(
        UserStringNop("SITREP_TECH_UNLOCKED"),
        current_turn,
        "icons/sitrep/tech_unlocked.png",
        UserStringNop("SITREP_TECH_UNLOCKED_LABEL"), true);
    sitrep.AddVariable(VarText::TECH_TAG, std::move(tech_name));
    return sitrep;
}

SitRepEntry CreatePolicyUnlockedSitRep(std::string policy_name, int current_turn) {
    SitRepEntry sitrep(
        UserStringNop("SITREP_POLICY_UNLOCKED"),
        current_turn + 1,
        "icons/sitrep/policy_unlocked.png",
        UserStringNop("SITREP_POLICY_UNLOCKED_LABEL"), true);
    sitrep.AddVariable(VarText::POLICY_TAG, std::move(policy_name));
    return sitrep;
}

SitRepEntry CreateBuildingTypeUnlockedSitRep(std::string building_type_name, int current_turn) {
    SitRepEntry sitrep(
        UserStringNop("SITREP_BUILDING_TYPE_UNLOCKED"),
        current_turn,
        "icons/sitrep/building_type_unlocked.png",
        UserStringNop("SITREP_BUILDING_TYPE_UNLOCKED_LABEL"), true);
    sitrep.AddVariable(VarText::BUILDING_TYPE_TAG, std::move(building_type_name));
    return sitrep;
}

SitRepEntry CreateShipHullUnlockedSitRep(std::string ship_hull_name, int current_turn) {
    SitRepEntry sitrep(
        UserStringNop("SITREP_SHIP_HULL_UNLOCKED"),
        current_turn,
        "icons/sitrep/ship_hull_unlocked.png",
        UserStringNop("SITREP_SHIP_HULL_UNLOCKED_LABEL"), true);
    sitrep.AddVariable(VarText::SHIP_HULL_TAG, std::move(ship_hull_name));
    return sitrep;
}

SitRepEntry CreateShipPartUnlockedSitRep(std::string ship_part_name, int current_turn) {
    SitRepEntry sitrep(
        UserStringNop("SITREP_SHIP_PART_UNLOCKED"),
        current_turn,
        "icons/sitrep/ship_part_unlocked.png",
        UserStringNop("SITREP_SHIP_PART_UNLOCKED_LABEL"), true);
    sitrep.AddVariable(VarText::SHIP_PART_TAG, std::move(ship_part_name));
    return sitrep;
}

SitRepEntry CreateCombatSitRep(int system_id, int log_id, int enemy_id, int current_turn) {
    std::string template_string = (enemy_id == ALL_EMPIRES)
        ? UserStringNop("SITREP_COMBAT_SYSTEM")
        : UserStringNop("SITREP_COMBAT_SYSTEM_ENEMY");
    std::string label_string = (enemy_id == ALL_EMPIRES)
        ? UserStringNop("SITREP_COMBAT_SYSTEM_LABEL")
        : UserStringNop("SITREP_COMBAT_SYSTEM_ENEMY_LABEL");
    SitRepEntry sitrep(
        std::move(template_string), current_turn + 1,
        "icons/sitrep/combat.png", std::move(label_string), true);
    sitrep.AddVariable(VarText::SYSTEM_ID_TAG, std::to_string(system_id));
    sitrep.AddVariable(VarText::COMBAT_ID_TAG, std::to_string(log_id));
    sitrep.AddVariable(VarText::EMPIRE_ID_TAG, std::to_string(enemy_id));
    return sitrep;
}

SitRepEntry CreateGroundCombatSitRep(int planet_id, int enemy_id, int current_turn) {
    std::string template_string = (enemy_id == ALL_EMPIRES)
        ? UserStringNop("SITREP_GROUND_BATTLE")
        : UserStringNop("SITREP_GROUND_BATTLE_ENEMY");
    std::string label_string = (enemy_id == ALL_EMPIRES)
        ? UserStringNop("SITREP_GROUND_BATTLE_LABEL")
        : UserStringNop("SITREP_GROUND_BATTLE_ENEMY_LABEL");
    SitRepEntry sitrep(
        std::move(template_string), current_turn + 1,
        "icons/sitrep/ground_combat.png", std::move(label_string), true);
    sitrep.AddVariable(VarText::PLANET_ID_TAG, std::to_string(planet_id));
    sitrep.AddVariable(VarText::EMPIRE_ID_TAG, std::to_string(enemy_id));
    return sitrep;
}

SitRepEntry CreatePlanetCapturedSitRep(int planet_id, int empire_id, int current_turn) {
    SitRepEntry sitrep(
        UserStringNop("SITREP_PLANET_CAPTURED"),
        current_turn + 1,
        "icons/sitrep/planet_captured.png",
        UserStringNop("SITREP_PLANET_CAPTURED_LABEL"), true);
    sitrep.AddVariable(VarText::PLANET_ID_TAG, std::to_string(planet_id));
    sitrep.AddVariable(VarText::EMPIRE_ID_TAG, std::to_string(empire_id));
    return sitrep;
}

SitRepEntry CreatePlanetRebelledSitRep(int planet_id, int empire_id, int current_turn) {
    SitRepEntry sitrep(
        UserStringNop("SITREP_PLANET_CAPTURED_NEUTRALS"),
        current_turn + 1,
        "icons/sitrep/planet_captured.png",
        UserStringNop("SITREP_PLANET_CAPTURED_NEUTRALS_LABEL"), true);
    sitrep.AddVariable(VarText::PLANET_ID_TAG, std::to_string(planet_id));
    sitrep.AddVariable(VarText::EMPIRE_ID_TAG, std::to_string(empire_id));
    return sitrep;
}

namespace {
    SitRepEntry GenericCombatDamagedObjectSitrep(int combat_system_id, int current_turn) {
        SitRepEntry sitrep(
            UserStringNop("SITREP_OBJECT_DAMAGED_AT_SYSTEM"),
            current_turn + 1,
            "icons/sitrep/combat_damage.png",
            UserStringNop("SITREP_OBJECT_DAMAGED_AT_SYSTEM_LABEL"), true);
        sitrep.AddVariable(VarText::SYSTEM_ID_TAG, std::to_string(combat_system_id));
        return sitrep;
    }

    SitRepEntry GenericCombatDestroyedObjectSitrep(int combat_system_id, int current_turn) {
        SitRepEntry sitrep(
            UserStringNop("SITREP_OBJECT_DESTROYED_AT_SYSTEM"),
            current_turn + 1,
            "icons/sitrep/combat_destroyed.png",
            UserStringNop("SITREP_OBJECT_DESTROYED_AT_SYSTEM_LABEL"), true);
        sitrep.AddVariable(VarText::SYSTEM_ID_TAG, std::to_string(combat_system_id));
        return sitrep;
    }
}

SitRepEntry CreateCombatDamagedObjectSitRep(const UniverseObject* obj, int combat_system_id,
                                            int empire_id, int current_turn)
{
    if (!obj)
        return GenericCombatDamagedObjectSitrep(combat_system_id, current_turn);
    const int object_id = obj->ID();

    SitRepEntry sitrep;

    if (auto ship = dynamic_cast<const Ship*>(obj)) {
        if (ship->Unowned())
            sitrep = SitRepEntry(
                UserStringNop("SITREP_UNOWNED_SHIP_DAMAGED_AT_SYSTEM"),
                current_turn + 1,
                "icons/sitrep/combat_damage.png",
                UserStringNop("SITREP_UNOWNED_SHIP_DAMAGED_AT_SYSTEM_LABEL"), true);
        else
            sitrep = SitRepEntry(
                UserStringNop("SITREP_SHIP_DAMAGED_AT_SYSTEM"),
                current_turn + 1,
                "icons/sitrep/combat_damage.png",
                UserStringNop("SITREP_SHIP_DAMAGED_AT_SYSTEM_LABEL"), true);
        sitrep.AddVariable(VarText::SHIP_ID_TAG,   std::to_string(object_id));
        sitrep.AddVariable(VarText::DESIGN_ID_TAG, std::to_string(ship->DesignID()));

    } else if (auto planet = dynamic_cast<const Planet*>(obj)) {
        if (planet->Unowned())
            sitrep = SitRepEntry(
                UserStringNop("SITREP_UNOWNED_PLANET_ATTACKED_AT_SYSTEM"),
                current_turn + 1,
                "icons/sitrep/colony_bombarded.png",
                UserStringNop("SITREP_UNOWNED_PLANET_ATTACKED_AT_SYSTEM_LABEL"), true);
        else
            sitrep = SitRepEntry(
                UserStringNop("SITREP_PLANET_ATTACKED_AT_SYSTEM"),
                current_turn + 1,
                "icons/sitrep/colony_bombarded.png",
                UserStringNop("SITREP_PLANET_ATTACKED_AT_SYSTEM_LABEL"), true);
        sitrep.AddVariable(VarText::PLANET_ID_TAG, std::to_string(object_id));

    } else {
        sitrep = GenericCombatDestroyedObjectSitrep(combat_system_id, current_turn);
    }

    sitrep.AddVariable(VarText::EMPIRE_ID_TAG, std::to_string(obj->Owner()));
    sitrep.AddVariable(VarText::SYSTEM_ID_TAG, std::to_string(combat_system_id));

    return sitrep;
}

SitRepEntry CreateCombatDestroyedObjectSitRep(const UniverseObject* obj, int combat_system_id,
                                              int empire_id, int current_turn)
{
    if (!obj) {
        DebugLogger() << "CreateCombatDestroyedObjectSitRep: passed null object";
        return GenericCombatDestroyedObjectSitrep(combat_system_id, current_turn);
    }
    const int object_id = obj->ID();

    SitRepEntry sitrep;

    switch (obj->ObjectType()) {
    case UniverseObjectType::OBJ_SHIP: {
        auto ship = static_cast<const Ship*>(obj);
        if (ship->Unowned())
            sitrep = SitRepEntry(
                UserStringNop("SITREP_UNOWNED_SHIP_DESTROYED_AT_SYSTEM"),
                current_turn + 1,
                "icons/sitrep/combat_destroyed.png",
                UserStringNop("SITREP_UNOWNED_SHIP_DESTROYED_AT_SYSTEM_LABEL"), true);
        else if (ship->OwnedBy(empire_id))
            sitrep = SitRepEntry(
                UserStringNop("SITREP_OWN_SHIP_DESTROYED_AT_SYSTEM"),
                current_turn + 1,
                "icons/sitrep/combat_destroyed.png",
                UserStringNop("SITREP_OWN_SHIP_DESTROYED_AT_SYSTEM_LABEL"), true);
        else
            sitrep = SitRepEntry(
                UserStringNop("SITREP_SHIP_DESTROYED_AT_SYSTEM"),
                current_turn + 1,
                "icons/sitrep/combat_destroyed.png",
                UserStringNop("SITREP_SHIP_DESTROYED_AT_SYSTEM_LABEL"), true);
        sitrep.AddVariable(VarText::SHIP_ID_TAG,   std::to_string(object_id));
        sitrep.AddVariable(VarText::DESIGN_ID_TAG, std::to_string(ship->DesignID()));
        break;
    }
    case UniverseObjectType::OBJ_FLEET: {
        auto fleet = static_cast<const Fleet*>(obj);
        if (fleet->Unowned())
            sitrep = SitRepEntry(
                UserStringNop("SITREP_UNOWNED_FLEET_DESTROYED_AT_SYSTEM"),
                current_turn + 1,
                "icons/sitrep/combat_destroyed.png",
                UserStringNop("SITREP_UNOWNED_FLEET_DESTROYED_AT_SYSTEM_LABEL"), true);
        else
            sitrep = SitRepEntry(
                UserStringNop("SITREP_FLEET_DESTROYED_AT_SYSTEM"),
                current_turn + 1,
                "icons/sitrep/combat_destroyed.png",
                UserStringNop("SITREP_FLEET_DESTROYED_AT_SYSTEM_LABEL"), true);
        sitrep.AddVariable(VarText::FLEET_ID_TAG, std::to_string(object_id));
        break;
    }
    case UniverseObjectType::OBJ_PLANET: {
        auto planet = static_cast<const Planet*>(obj);
        if (planet->Unowned())
            sitrep = SitRepEntry(
                UserStringNop("SITREP_UNOWNED_PLANET_DESTROYED_AT_SYSTEM"),
                current_turn + 1,
                "icons/sitrep/combat_destroyed.png",
                UserStringNop("SITREP_UNOWNED_PLANET_DESTROYED_AT_SYSTEM_LABEL"), true);
        else
            sitrep = SitRepEntry(
                UserStringNop("SITREP_PLANET_DESTROYED_AT_SYSTEM"),
                current_turn + 1,
                "icons/sitrep/combat_destroyed.png",
                UserStringNop("SITREP_PLANET_DESTROYED_AT_SYSTEM_LABEL"), true);
        sitrep.AddVariable(VarText::PLANET_ID_TAG, std::to_string(object_id));
        break;
    }
    case UniverseObjectType::OBJ_BUILDING: {
        auto building = static_cast<const Building*>(obj);
        if (building->Unowned())
            sitrep = SitRepEntry(
                UserStringNop("SITREP_UNOWNED_BUILDING_DESTROYED_ON_PLANET_AT_SYSTEM"),
                current_turn + 1,
                "icons/sitrep/combat_destroyed.png",
                UserStringNop("SITREP_UNOWNED_BUILDING_DESTROYED_ON_PLANET_AT_SYSTEM_LABEL"), true);
        else
            sitrep = SitRepEntry(
                UserStringNop("SITREP_BUILDING_DESTROYED_ON_PLANET_AT_SYSTEM"),
                current_turn + 1,
                "icons/sitrep/combat_destroyed.png",
                UserStringNop("SITREP_BUILDING_DESTROYED_ON_PLANET_AT_SYSTEM_LABEL"), true);
        sitrep.AddVariable(VarText::BUILDING_ID_TAG, std::to_string(object_id));
        sitrep.AddVariable(VarText::PLANET_ID_TAG,   std::to_string(building->PlanetID()));
        break;
    }
    default:
        sitrep = GenericCombatDestroyedObjectSitrep(combat_system_id, current_turn);
    }

    sitrep.AddVariable(VarText::EMPIRE_ID_TAG, std::to_string(obj->Owner()));
    sitrep.AddVariable(VarText::SYSTEM_ID_TAG, std::to_string(combat_system_id));

    return sitrep;
}

SitRepEntry CreatePlanetDepopulatedSitRep(int planet_id, int current_turn) {
    SitRepEntry sitrep(
        UserStringNop("SITREP_PLANET_DEPOPULATED"),
        current_turn + 1,
        "icons/sitrep/colony_destroyed.png",
        UserStringNop("SITREP_PLANET_DEPOPULATED_LABEL"), true);
    sitrep.AddVariable(VarText::PLANET_ID_TAG,     std::to_string(planet_id));
    return sitrep;
}

SitRepEntry CreatePlanetAnnexedSitRep(int planet_id, int original_owner_id, int annexer_empire_id, int current_turn) {
    static constexpr std::string_view neutral_annex_txt = UserStringNop("SITREP_PLANET_ANNEXED");
    static constexpr std::string_view other_empire_annex_txt = UserStringNop("SITREP_PLANET_ANNEXED_FROM_OTHER_EMPIRE");
    static constexpr std::string_view neutral_annex_label = UserStringNop("SITREP_PLANET_ANNEXED_LABEL");
    static constexpr std::string_view other_empire_annex_label = UserStringNop("SITREP_PLANET_ANNEXED_FROM_OTHER_EMPIRE_LABEL");

    const auto msg = (original_owner_id == ALL_EMPIRES) ? neutral_annex_txt : other_empire_annex_txt;
    const auto label = (original_owner_id == ALL_EMPIRES) ? neutral_annex_label : other_empire_annex_label;
    SitRepEntry sitrep{msg.data(), current_turn + 1, "icons/sitrep/annexed.png", label.data(), true};
    sitrep.AddVariable(VarText::PLANET_ID_TAG,  std::to_string(planet_id));
    sitrep.AddVariable("annexer",               std::to_string(annexer_empire_id));
    if (original_owner_id != ALL_EMPIRES)
        sitrep.AddVariable("original",          std::to_string(original_owner_id));
    return sitrep;
}

SitRepEntry CreatePlanetColonizedSitRep(int planet_id, std::string species, int current_turn) {
    SitRepEntry sitrep(
        UserStringNop("SITREP_PLANET_COLONIZED"),
        current_turn + 1,
        "icons/sitrep/planet_colonized.png",
        UserStringNop("SITREP_PLANET_COLONIZED_LABEL"), true);
    sitrep.AddVariable(VarText::PLANET_ID_TAG,  std::to_string(planet_id));
    sitrep.AddVariable(VarText::SPECIES_TAG,    std::move(species));
    return sitrep;
}

SitRepEntry CreatePlanetOutpostedSitRep(int planet_id, int current_turn) {
    SitRepEntry sitrep(
        UserStringNop("SITREP_PLANET_OUTPOSTED"),
        current_turn + 1,
        "icons/sitrep/planet_colonized.png",
        UserStringNop("SITREP_PLANET_OUTPOSTED_LABEL"), true);
    sitrep.AddVariable(VarText::PLANET_ID_TAG,     std::to_string(planet_id));
    return sitrep;
}

SitRepEntry CreatePlanetEstablishFailedSitRep(int planet_id, int ship_id, int current_turn) {
    SitRepEntry sitrep(
        UserStringNop("SITREP_PLANET_ESTABLISH_FAILED"),
        current_turn + 1,
        "icons/sitrep/planet_colonized.png",
        UserStringNop("SITREP_PLANET_ESTABLISH_FAILED_LABEL"), true);
    sitrep.AddVariable(VarText::PLANET_ID_TAG,     std::to_string(planet_id));
    sitrep.AddVariable(VarText::SHIP_ID_TAG,       std::to_string(ship_id));
    return sitrep;
}

SitRepEntry CreatePlanetEstablishFailedVisibleOtherSitRep(int planet_id, int ship_id, int other_empire_id,
                                                          int current_turn)
{
    SitRepEntry sitrep(
        UserStringNop("SITREP_PLANET_ESTABLISH_FAILED_VISIBLE_OTHER"),
        current_turn + 1,
        "icons/sitrep/planet_colonized.png",
        UserStringNop("SITREP_PLANET_ESTABLISH_FAILED_VISIBLE_OTHER_LABEL"), true);
    sitrep.AddVariable(VarText::PLANET_ID_TAG,     std::to_string(planet_id));
    sitrep.AddVariable(VarText::SHIP_ID_TAG,       std::to_string(ship_id));
    sitrep.AddVariable(VarText::EMPIRE_ID_TAG,     std::to_string(other_empire_id));
    return sitrep;
}

SitRepEntry CreatePlanetEstablishFailedArmedSitRep(int planet_id, int ship_id, int other_empire_id,
                                                   int current_turn)
{
    if (other_empire_id == ALL_EMPIRES) {
        SitRepEntry sitrep(
            UserStringNop("SITREP_PLANET_ESTABLISH_FAILED_NEUTRAL_ARMED"),
            current_turn + 1,
            "icons/sitrep/planet_colonized.png",
            UserStringNop("SITREP_PLANET_ESTABLISH_FAILED_ARMED_NEUTRAL_LABEL"), true);
        sitrep.AddVariable(VarText::PLANET_ID_TAG,     std::to_string(planet_id));
        sitrep.AddVariable(VarText::SHIP_ID_TAG,       std::to_string(ship_id));
        return sitrep;

    } else {
        SitRepEntry sitrep(
            UserStringNop("SITREP_PLANET_ESTABLISH_FAILED_ARMED"),
            current_turn + 1,
            "icons/sitrep/planet_colonized.png",
            UserStringNop("SITREP_PLANET_ESTABLISH_FAILED_ARMED_LABEL"), true);
        sitrep.AddVariable(VarText::PLANET_ID_TAG,     std::to_string(planet_id));
        sitrep.AddVariable(VarText::SHIP_ID_TAG,       std::to_string(ship_id));
        sitrep.AddVariable(VarText::EMPIRE_ID_TAG,     std::to_string(other_empire_id));
        return sitrep;
    }
}

SitRepEntry CreatePlanetGiftedSitRep(int planet_id, int empire_id, int current_turn) {
    SitRepEntry sitrep(
        UserStringNop("SITREP_PLANET_GIFTED"),
        current_turn + 1,
        "icons/sitrep/gift.png",
        UserStringNop("SITREP_PLANET_GIFTED_LABEL"), true);
    sitrep.AddVariable(VarText::PLANET_ID_TAG,  std::to_string(planet_id));
    sitrep.AddVariable(VarText::EMPIRE_ID_TAG,  std::to_string(empire_id));
    return sitrep;
}

SitRepEntry CreateFleetGiftedSitRep(int fleet_id, int empire_id, int current_turn) {
    SitRepEntry sitrep(
        UserStringNop("SITREP_FLEET_GIFTED"),
        current_turn + 1,
        "icons/sitrep/gift.png",
        UserStringNop("SITREP_FLEET_GIFTED_LABEL"), true);
    sitrep.AddVariable(VarText::FLEET_ID_TAG,   std::to_string(fleet_id));
    sitrep.AddVariable(VarText::EMPIRE_ID_TAG,  std::to_string(empire_id));
    return sitrep;
}

SitRepEntry CreateFleetArrivedAtDestinationSitRep(int system_id, int fleet_id, int recipient_empire_id,
                                                  const ScriptingContext& context)
{
    const ObjectMap& o = context.ContextObjects();
    const Universe& u = context.ContextUniverse();

    const auto fleet = o.get<Fleet>(fleet_id);

    //bool system_contains_recipient_empire_planets = false;
    //if (const System* system = o.get<System>(system_id)) {
    //    for (const auto& planet : system->all<Planet>()) {
    //        if (!planet || planet->Unowned())
    //            continue;
    //        if (planet->OwnedBy(recipient_empire_id)) {
    //            system_contains_recipient_empire_planets = true;
    //            break;
    //        }
    //    }
    //}

    //There are variants of this message for {monster, own, foreign} * {one ship, fleet}.
    // TODO: More variants for systems with / without recipient-owned planets
    //These should really be assembled from several pieces: a fleet description,
    //a system description, and a message template into which both are substituted.
    if (!fleet) {
        SitRepEntry sitrep(
            UserStringNop("SITREP_FLEET_ARRIVED_AT_SYSTEM"),
            context.current_turn + 1,
            "icons/sitrep/fleet_arrived.png",
            UserStringNop("SITREP_FLEET_ARRIVED_AT_SYSTEM_LABEL"), true);
        sitrep.AddVariable(VarText::SYSTEM_ID_TAG,  std::to_string(system_id));
        sitrep.AddVariable(VarText::FLEET_ID_TAG,   std::to_string(fleet_id));
        return sitrep;

    } else if (fleet->Unowned() && fleet->HasMonsters(u)) {
        if (fleet->NumShips() == 1) {
            SitRepEntry sitrep(
                UserStringNop("SITREP_MONSTER_SHIP_ARRIVED_AT_DESTINATION"),
                context.current_turn + 1,
                "icons/sitrep/fleet_arrived.png",
                UserStringNop("SITREP_MONSTER_SHIP_ARRIVED_AT_DESTINATION_LABEL"), true);
            sitrep.AddVariable(VarText::SYSTEM_ID_TAG,     std::to_string(system_id));
            sitrep.AddVariable(VarText::FLEET_ID_TAG,      std::to_string(fleet_id));
            int ship_id = *fleet->ShipIDs().begin();
            sitrep.AddVariable(VarText::SHIP_ID_TAG,       std::to_string(ship_id));
            if (auto ship = o.get<Ship>(ship_id))
                sitrep.AddVariable(VarText::DESIGN_ID_TAG, std::to_string(ship->DesignID()));
            return sitrep;
        } else {
            SitRepEntry sitrep(
                UserStringNop("SITREP_MONSTER_FLEET_ARRIVED_AT_DESTINATION"),
                context.current_turn + 1,
                "icons/sitrep/fleet_arrived.png",
                UserStringNop("SITREP_MONSTER_FLEET_ARRIVED_AT_DESTINATION_LABEL"), true);
            sitrep.AddVariable(VarText::SYSTEM_ID_TAG,  std::to_string(system_id));
            sitrep.AddVariable(VarText::FLEET_ID_TAG,   std::to_string(fleet_id));
            sitrep.AddVariable(VarText::RAW_TEXT_TAG,   std::to_string(fleet->NumShips()));
            return sitrep;
        }

    } else if (fleet->Unowned()) {
        SitRepEntry sitrep(
            UserStringNop("SITREP_FLEET_ARRIVED_AT_DESTINATION"),
            context.current_turn + 1,
            "icons/sitrep/fleet_arrived.png",
            UserStringNop("SITREP_FLEET_ARRIVED_AT_DESTINATION_LABEL"), true);
        sitrep.AddVariable(VarText::SYSTEM_ID_TAG,  std::to_string(system_id));
        sitrep.AddVariable(VarText::FLEET_ID_TAG,   std::to_string(fleet_id));
        sitrep.AddVariable(VarText::RAW_TEXT_TAG,   std::to_string(fleet->NumShips()));
        return sitrep;

    } else if (fleet->OwnedBy(recipient_empire_id)) {
        if (fleet->NumShips() == 1) {
            SitRepEntry sitrep(
                UserStringNop("SITREP_OWN_SHIP_ARRIVED_AT_DESTINATION"),
                context.current_turn + 1,
                "icons/sitrep/fleet_arrived.png",
                UserStringNop("SITREP_OWN_SHIP_ARRIVED_AT_DESTINATION_LABEL"), true);
            sitrep.AddVariable(VarText::SYSTEM_ID_TAG,     std::to_string(system_id));
            sitrep.AddVariable(VarText::FLEET_ID_TAG,      std::to_string(fleet_id));
            sitrep.AddVariable(VarText::EMPIRE_ID_TAG,     std::to_string(fleet->Owner()));
            const int ship_id = *fleet->ShipIDs().begin();
            sitrep.AddVariable(VarText::SHIP_ID_TAG,       std::to_string(ship_id));
            if (auto ship = o.get<Ship>(ship_id))
                sitrep.AddVariable(VarText::DESIGN_ID_TAG, std::to_string(ship->DesignID()));
            return sitrep;
        } else {
            SitRepEntry sitrep(
                UserStringNop("SITREP_OWN_FLEET_ARRIVED_AT_DESTINATION"),
                context.current_turn + 1,
                "icons/sitrep/fleet_arrived.png",
                UserStringNop("SITREP_OWN_FLEET_ARRIVED_AT_DESTINATION_LABEL"), true);
            sitrep.AddVariable(VarText::SYSTEM_ID_TAG,  std::to_string(system_id));
            sitrep.AddVariable(VarText::FLEET_ID_TAG,   std::to_string(fleet_id));
            sitrep.AddVariable(VarText::EMPIRE_ID_TAG,  std::to_string(fleet->Owner()));
            sitrep.AddVariable(VarText::RAW_TEXT_TAG,   std::to_string(fleet->NumShips()));
            return sitrep;
        }

    } else if (fleet->NumShips() == 1) {
        SitRepEntry sitrep(
            UserStringNop("SITREP_FOREIGN_SHIP_ARRIVED_AT_DESTINATION"),
            context.current_turn + 1,
            "icons/sitrep/fleet_arrived.png",
            UserStringNop("SITREP_FOREIGN_SHIP_ARRIVED_AT_DESTINATION_LABEL"), true);
        sitrep.AddVariable(VarText::SYSTEM_ID_TAG,     std::to_string(system_id));
        sitrep.AddVariable(VarText::FLEET_ID_TAG,      std::to_string(fleet_id));
        sitrep.AddVariable(VarText::EMPIRE_ID_TAG,     std::to_string(fleet->Owner()));
        const int ship_id = *fleet->ShipIDs().begin();
        sitrep.AddVariable(VarText::SHIP_ID_TAG,       std::to_string(ship_id));
        if (auto ship = o.getRaw<Ship>(ship_id))
            sitrep.AddVariable(VarText::DESIGN_ID_TAG, std::to_string(ship->DesignID()));
        return sitrep;

    } else {
        SitRepEntry sitrep(
            UserStringNop("SITREP_FOREIGN_FLEET_ARRIVED_AT_DESTINATION"),
            context.current_turn + 1,
            "icons/sitrep/fleet_arrived.png",
            UserStringNop("SITREP_FOREIGN_FLEET_ARRIVED_AT_DESTINATION_LABEL"), true);
        sitrep.AddVariable(VarText::SYSTEM_ID_TAG,  std::to_string(system_id));
        sitrep.AddVariable(VarText::FLEET_ID_TAG,   std::to_string(fleet_id));
        sitrep.AddVariable(VarText::EMPIRE_ID_TAG,  std::to_string(fleet->Owner()));
        sitrep.AddVariable(VarText::RAW_TEXT_TAG,   std::to_string(fleet->NumShips()));
        return sitrep;
    }
}

SitRepEntry CreateFleetBlockadedSitRep(int system_id, int blockaded_fleet_id, int blockaded_empire_id,
                                       int blockading_empire_id, const ScriptingContext& context)
{
    const auto template_label = (blockaded_empire_id != ALL_EMPIRES) ?
        std::pair{UserStringNop("SITREP_FLEET_BLOCKADED"), UserStringNop("SITREP_FLEET_BLOCKADED_LABEL")} :
        std::pair{UserStringNop("SITREP_FLEET_BLOCKADED_NEUTRAL"), UserStringNop("SITREP_FLEET_BLOCKADED_LABEL")};

    SitRepEntry sitrep(
        template_label.first,
        context.current_turn + 1,
        "icons/sitrep/blockade.png",
        template_label.second,
        true);
    sitrep.AddVariable(VarText::SYSTEM_ID_TAG,  std::to_string(system_id));
    sitrep.AddVariable(VarText::FLEET_ID_TAG,   std::to_string(blockaded_fleet_id));
    sitrep.AddVariable("blockaded",             std::to_string(blockaded_empire_id));
    //if (blockading_empire_id != ALL_EMPIRES)
        sitrep.AddVariable("blockader",        std::to_string(blockading_empire_id));
    return sitrep;
}

SitRepEntry CreateFleetBlockadedSitRep(int system_id, int blockaded_fleet_id,
                                       int blockaded_empire_id, const ScriptingContext& context)
{
    SitRepEntry sitrep(
        UserStringNop("SITREP_FLEET_BLOCKADED_NO_EMPIRE"),
        context.current_turn + 1,
        "icons/sitrep/blockade.png",
        UserStringNop("SITREP_FLEET_BLOCKADED_NO_EMPIRE_LABEL"), true);
    sitrep.AddVariable(VarText::SYSTEM_ID_TAG, std::to_string(system_id));
    sitrep.AddVariable(VarText::FLEET_ID_TAG,  std::to_string(blockaded_fleet_id));
    sitrep.AddVariable(VarText::EMPIRE_ID_TAG, std::to_string(blockaded_empire_id));
    return sitrep;
}

SitRepEntry CreateEmpireEliminatedSitRep(int empire_id, int current_turn) {
    SitRepEntry sitrep(
        UserStringNop("SITREP_EMPIRE_ELIMINATED"),
        current_turn + 1,
        "icons/sitrep/empire_eliminated.png",
        UserStringNop("SITREP_EMPIRE_ELIMINATED_LABEL"), true);
    sitrep.AddVariable(VarText::EMPIRE_ID_TAG, std::to_string(empire_id));
    return sitrep;
}

SitRepEntry CreateVictorySitRep(std::string reason_string, int empire_id, int current_turn) {
    SitRepEntry sitrep(std::move(reason_string), current_turn + 1,
                       "icons/sitrep/victory.png", UserStringNop("SITREP_VICTORY_LABEL"), true);
    sitrep.AddVariable(VarText::EMPIRE_ID_TAG, std::to_string(empire_id));
    return sitrep;
}

SitRepEntry CreateSitRep(std::string template_string, int turn, std::string icon,
                         std::vector<std::pair<std::string, std::string>> parameters,
                         std::string label, bool stringtable_lookup)
{
    SitRepEntry sitrep(std::move(template_string), turn, std::move(icon), std::move(label), stringtable_lookup);
    sitrep.AddVariables(std::move(parameters));
    return sitrep;
}
