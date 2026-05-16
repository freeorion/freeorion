#ifndef COMBATEVENTS_H
#define COMBATEVENTS_H


#include <map>
#include <memory>
#include <set>

#include <tuple>

#include "../util/Export.h"
#include "../universe/ConstantsFwd.h"
#include "../universe/EnumsFwd.h"
#include "../universe/UniverseObject.h"
#include "../universe/Universe.h"

#include "CombatEvent.h"

#if !defined(CONSTEXPR_STRING)
#  if defined(__cpp_lib_constexpr_string) && ((!defined(__GNUC__) || (__GNUC__ > 11))) && ((!defined(_MSC_VER) || (_MSC_VER >= 1934)))
#    define CONSTEXPR_STRING constexpr
#  else
#    define CONSTEXPR_STRING
#  endif
#endif
#if !defined(CONSTEXPR_VEC)
#  if defined(__cpp_lib_constexpr_vector)
#    define CONSTEXPR_VEC constexpr
#  else
#    define CONSTEXPR_VEC
#  endif
#endif

namespace CombatEventDetail {
    constexpr bool nxcepv = noexcept(std::vector<CombatEventPtr>{});
    constexpr bool nxstr = noexcept(std::string{});
    constexpr bool nxstrmove = noexcept(std::string{std::declval<std::string&&>()});
}



/** SimultaneousEvents describes a set of simultaneous events in one bout of combat.
  * It may have some sub-events and the sub-events will be shown in
  * viewing empire first followed by ALL_EMPIRES and then other empires.*/
struct FO_COMMON_API SimultaneousEvents : public CombatEvent {
    [[nodiscard]] CONSTEXPR_VEC SimultaneousEvents() noexcept(CombatEventDetail::nxcepv) = default;

    void AddEvent(CombatEventPtr event) { events.push_back(std::move(event)); }

    [[nodiscard]] std::string DebugString(const ScriptingContext&) const override;
    [[nodiscard]] std::string CombatLogDescription(int, const ScriptingContext&)
        const noexcept(CombatEventDetail::nxstr) override { return {}; }
    [[nodiscard]] std::vector<const CombatEvent*> SubEvents(int viewing_empire_id) const override;
    [[nodiscard]] auto& Events() const noexcept { return events; };

    [[nodiscard]] bool IsEmpty() const noexcept override { return events.empty(); }
    [[nodiscard]] bool FlattenSubEvents() const noexcept override { return true; }

protected:
    std::vector<CombatEventPtr> events;

    template <typename Archive>
    friend void serialize(Archive&, SimultaneousEvents&, unsigned int const);
};


/** InitialStealthEvent describes the initially stealthy combatants.
  * Note:  Because it is initialized with the unfiltered stealth information it
  * contains information not availble to all empires. */
struct FO_COMMON_API InitialStealthEvent : public CombatEvent {
    [[nodiscard]] InitialStealthEvent() = default;
    [[nodiscard]] explicit InitialStealthEvent(EmpireObjectVisibilityMap x) noexcept :
        empire_object_visibility(std::move(x))
    {}

    [[nodiscard]] std::string DebugString(const ScriptingContext& context) const override;
    [[nodiscard]] std::string CombatLogDescription(int viewing_empire_id, const ScriptingContext& context) const override;

private:
    EmpireObjectVisibilityMap empire_object_visibility;

    template <typename Archive>
    friend void serialize(Archive&, InitialStealthEvent&, unsigned int const);
};


/** StealthChangeEvent describes changes in the visibility of objects during combat.
  * At this time always decloaking. */
struct FO_COMMON_API StealthChangeEvent : public CombatEvent {
    [[nodiscard]] StealthChangeEvent() = default;

    [[nodiscard]] std::string DebugString(const ScriptingContext& context) const override;
    [[nodiscard]] std::string CombatLogDescription(int viewing_empire_id, const ScriptingContext& context) const override;
    [[nodiscard]] std::vector<const CombatEvent*> SubEvents(int viewing_empire_id) const override;
    [[nodiscard]] bool IsEmpty() const noexcept override { return events.empty(); }

    void AddEvent(int attacker_id, int target_id, int attacker_empire_id, int target_empire_id, Visibility new_visibility)
    { events.emplace_back(attacker_id, target_id, attacker_empire_id, target_empire_id, new_visibility); }

    void AddEvent(int launcher_id, int launcher_empire_id, int observer_empire_id, Visibility new_visibility)
    { events.emplace_back(launcher_id, launcher_empire_id, observer_empire_id, new_visibility); }

    struct StealthChangeEventDetail : public CombatEvent {
        [[nodiscard]] constexpr StealthChangeEventDetail() noexcept = default;
        [[nodiscard]] constexpr StealthChangeEventDetail(int attacker_id_, int target_id_, int attacker_empire_,
                                                         int target_observer_empire_, Visibility new_visibility_) noexcept :
            attacker_id(attacker_id_),
            target_id(target_id_),
            attacker_empire_id(attacker_empire_),
            target_observer_empire_id(target_observer_empire_),
            visibility(new_visibility_)
        {}
        [[nodiscard]] constexpr StealthChangeEventDetail(int laucher_id_, int attacker_laucher_empire_,
                                                         int target_observer_empire_, Visibility new_visibility_) noexcept :
            attacker_id(laucher_id_),
            attacker_empire_id(attacker_laucher_empire_),
            target_observer_empire_id(target_observer_empire_),
            visibility(new_visibility_),
            is_fighter_launch(true)
        {}

        [[nodiscard]] std::string DebugString(const ScriptingContext& context) const override;
        [[nodiscard]] std::string CombatLogDescription(int viewing_empire_id, const ScriptingContext& context) const override;

        int attacker_id = INVALID_OBJECT_ID;
        int target_id = INVALID_OBJECT_ID;
        int attacker_empire_id = ALL_EMPIRES;
        int target_observer_empire_id = ALL_EMPIRES;
        Visibility visibility = Visibility::VIS_NO_VISIBILITY;
        bool is_fighter_launch = false;
    };

private:
    std::vector<StealthChangeEventDetail> events;

    template <typename Archive>
    friend void serialize(Archive&, StealthChangeEvent&, unsigned int const);
};


/** An event that describes a single attack by one object or fighter against
  * another object or fighter. */
struct FO_COMMON_API WeaponFireEvent final : public CombatEvent {
    [[nodiscard]] CONSTEXPR_STRING WeaponFireEvent() noexcept(CombatEventDetail::nxstr) = default;

    /** WeaponFireEvent encodes a single attack by \p attacker_id owned
      * by \p attacker_owner_id on \p target_id with \p weapon_name of
      * \p power against \p shield doing \p damage.
      *
      * If \p shield is negative that implies the weapon is shield piercing.

      * The use of tuple in the constructor is to keep the number of parameters below 10 which
      * is the maximum that some compilers that emulate variadic templates support. */
    [[nodiscard]] CONSTEXPR_STRING WeaponFireEvent(int attacker_id_, int target_id_,
                                                   std::string weapon_name_,
                                                   const std::tuple<float, float, float>& power_shield_damage,
                                                   int attacker_owner_id_, int target_owner_id_)
        noexcept(CombatEventDetail::nxstrmove) :
        attacker_id(attacker_id_),
        target_id(target_id_),
        weapon_name(std::move(weapon_name_)),
        power(std::get<0>(power_shield_damage)),
        shield(std::get<1>(power_shield_damage)),
        damage(std::get<2>(power_shield_damage)),
        attacker_owner_id(attacker_owner_id_),
        target_owner_id(target_owner_id_)
    {}

    [[nodiscard]] std::string DebugString(const ScriptingContext& context) const override;
    [[nodiscard]] std::string CombatLogDescription(int viewing_empire_id, const ScriptingContext& context) const override;
    [[nodiscard]] std::string CombatLogDetails(int viewing_empire_id) const override;
    [[nodiscard]] bool IsEmpty() const noexcept override { return false; }
    [[nodiscard]] std::optional<int> PrincipalFaction(int viewing_empire_id) const noexcept override { return attacker_id; }

    int attacker_id = INVALID_OBJECT_ID;
    int target_id = INVALID_OBJECT_ID;
    std::string weapon_name;
    float power = 0.0f;
    float shield = 0.0f;
    float damage = 0.0f;
    int attacker_owner_id = ALL_EMPIRES;
    int target_owner_id = ALL_EMPIRES;
};


/** Summarizes incapacitations, ie. when an object becomes unable to fight any more.
  * eg. ship is destroyed. */
struct FO_COMMON_API IncapacitationsEvent : public CombatEvent {
    [[nodiscard]] CONSTEXPR_VEC IncapacitationsEvent() noexcept = default;
    [[nodiscard]] CONSTEXPR_VEC explicit IncapacitationsEvent(UniverseObjectType objects_type_) noexcept :
        objects_type(objects_type_)
    {}

    [[nodiscard]] std::string DebugString(const ScriptingContext& context) const override;
    [[nodiscard]] std::string CombatLogDescription(int viewing_empire_id, const ScriptingContext& context) const override;
    [[nodiscard]] std::vector<const CombatEvent*> SubEvents(int viewing_empire_id) const override;
    [[nodiscard]] bool IsEmpty() const noexcept override {
        static constexpr auto second_is_empty = [](const auto& c) noexcept { return c.second.empty(); };
        return events.empty() || std::all_of(events.begin(), events.end(), second_is_empty);
    }
    [[nodiscard]] bool FlattenSubEvents() const noexcept override { return true; }

    void AddEvent(int object_id, int owner_id);

    struct IncapacitationDetail : public CombatEvent {
        [[nodiscard]] constexpr IncapacitationDetail() noexcept = default;
        [[nodiscard]] constexpr explicit IncapacitationDetail(
            int id_, UniverseObjectType obj_type_ = UniverseObjectType::INVALID_UNIVERSE_OBJECT_TYPE) noexcept :
            id(id_), object_type(obj_type_)
        {}

        [[nodiscard]] std::string DebugString(const ScriptingContext& context) const override
        { return std::string{"incapacitation of id "} + std::to_string(id); }

        [[nodiscard]] std::string CombatLogDescription(int viewing_empire_id, const ScriptingContext& context) const override;

        [[nodiscard]] constexpr operator int() const noexcept { return id; }

        int id = INVALID_OBJECT_ID;
        UniverseObjectType object_type = UniverseObjectType::INVALID_UNIVERSE_OBJECT_TYPE;
    };

private:
    std::vector<std::pair<int, std::vector<IncapacitationDetail>>> events;
    UniverseObjectType objects_type = UniverseObjectType::INVALID_UNIVERSE_OBJECT_TYPE;

    template <typename Archive>
    friend void serialize(Archive&, IncapacitationsEvent&, unsigned int const);
};


/** FightersAttackFightersEvent aggregates all the fighter on fighter combat for one bout.*/
struct FO_COMMON_API FightersAttackFightersEvent : public CombatEvent {
    [[nodiscard]] FightersAttackFightersEvent() = default;

    [[nodiscard]] std::string DebugString(const ScriptingContext& context) const override;
    [[nodiscard]] std::string CombatLogDescription(int viewing_empire_id, const ScriptingContext& context) const override;
    [[nodiscard]] bool IsEmpty() const noexcept override { return events.empty(); }

    void AddEvent(int attacker_empire_, int target_empire_)
    { events[{attacker_empire_, target_empire_}] += 1; }

private:
    // Store the number of each of the identical fighter combat events.
    std::map<std::pair<int, int>, unsigned int> events;

    template <typename Archive>
    friend void serialize(Archive&, FightersAttackFightersEvent&, unsigned int const);
};


/** Created when an fighter is launched. */
struct FO_COMMON_API FighterLaunchEvent : public CombatEvent {
    [[nodiscard]] constexpr FighterLaunchEvent() noexcept = default;
    [[nodiscard]] constexpr FighterLaunchEvent(int launched_from_id_, int fighter_owner_empire_id_, int number_launched_) noexcept :
        fighter_owner_empire_id(fighter_owner_empire_id_),
        launched_from_id(launched_from_id_),
        number_launched(number_launched_)
    {}

    [[nodiscard]] std::string DebugString(const ScriptingContext& context) const override;
    [[nodiscard]] std::string CombatLogDescription(int viewing_empire_id, const ScriptingContext& context) const override;
    [[nodiscard]] std::optional<int> PrincipalFaction(int viewing_empire_id) const noexcept override { return fighter_owner_empire_id; }
    [[nodiscard]] bool IsEmpty() const noexcept override { return false; }

    int fighter_owner_empire_id = ALL_EMPIRES;
    int launched_from_id = INVALID_OBJECT_ID;
    int number_launched = 0;
};


/** FightersDestroyedEvent aggregates all the fighters destroyed during one combat bout.*/
struct FO_COMMON_API FightersDestroyedEvent : public CombatEvent {
    [[nodiscard]] FightersDestroyedEvent() = default;

    [[nodiscard]] std::string DebugString(const ScriptingContext& context) const override;
    [[nodiscard]] std::string CombatLogDescription(int viewing_empire_id, const ScriptingContext& context) const override;
    [[nodiscard]] bool IsEmpty() const noexcept override { return true; }

    void AddEvent(int target_empire_) { events[target_empire_] += 1; }

private:
    // for each empire ID that owned fighter(s), the number of destroyed fighters
    std::map<int, unsigned int> events;

    template <typename Archive>
    friend void serialize(Archive&, FightersDestroyedEvent&, unsigned int const);
};


/** WeaponsPlatformEvent describes a ship or planet with zero or more weapons firing its weapons in combat.
  * It may have some WeaponFire sub-events.*/
struct FO_COMMON_API WeaponsPlatformEvent : public CombatEvent {
    [[nodiscard]] WeaponsPlatformEvent() = default;
    [[nodiscard]] WeaponsPlatformEvent(int attacker_id_, int attacker_owner_id_) :
        attacker_id(attacker_id_),
        attacker_owner_id(attacker_owner_id_)
    {}

    void AddEvent(int target_id, int target_owner_id_, std::string weapon_name_,
                  float power_, float shield_, float damage_)
    {
        events[target_id].emplace_back(attacker_id, target_id, std::move(weapon_name_),
                                       std::tie(power_, shield_, damage_), attacker_owner_id, target_owner_id_);
    }

    [[nodiscard]] std::string DebugString(const ScriptingContext& context) const override;
    [[nodiscard]] std::string CombatLogDescription(int viewing_empire_id, const ScriptingContext& context) const override;
    [[nodiscard]] std::vector<const CombatEvent*> SubEvents(int viewing_empire_id) const override;
    [[nodiscard]] bool IsEmpty() const noexcept override { return events.empty(); }
    [[nodiscard]] std::optional<int> PrincipalFaction(int viewing_empire_id) const noexcept override { return attacker_owner_id; }

    int attacker_id = INVALID_OBJECT_ID;
    int attacker_owner_id = ALL_EMPIRES;
    std::map<int, std::vector<WeaponFireEvent>> events;

private:
    template <typename Archive>
    friend void serialize(Archive&, WeaponsPlatformEvent&, unsigned int const);
};


/** BoutEvent describes all the events that happen in one bout of combat. */
struct FO_COMMON_API BoutEvent : public CombatEvent {
    [[nodiscard]] explicit BoutEvent() noexcept(CombatEventDetail::nxcepv) = default;
    [[nodiscard]] explicit BoutEvent(int bout_) noexcept(CombatEventDetail::nxcepv) :
        bout(bout_)
    {}

    [[nodiscard]] std::string DebugString(const ScriptingContext& context) const override;
    [[nodiscard]] std::string CombatLogDescription(int viewing_empire_id, const ScriptingContext& context) const override;
    [[nodiscard]] std::vector<const CombatEvent*> SubEvents(int viewing_empire_id) const override;
    [[nodiscard]] bool FlattenSubEvents() const noexcept override { return true; }

    [[nodiscard]] bool IsEmpty() const noexcept override {
        return weapon_firings.IsEmpty() &&              weapons_platform_firings.IsEmpty() &&
               fighter_launches.IsEmpty() &&            fighters_destroyed.IsEmpty() &&
               fighters_attack_fighters.IsEmpty() &&    ship_incapacitations.IsEmpty() &&
               planet_incapacitations.IsEmpty() &&      other_incapacitations.IsEmpty();
    }

    int bout = 0;

    SimultaneousEvents weapon_firings;              // make into vector<WeaponFireEvent> ?
    SimultaneousEvents weapons_platform_firings;    // make info vector<WeaponsPlatformEvent> ?
    SimultaneousEvents fighter_launches;            // make info vector<FighterLaunchEvent> or fancier nested structure
    FightersDestroyedEvent fighters_destroyed;
    FightersAttackFightersEvent fighters_attack_fighters;
    IncapacitationsEvent ship_incapacitations{UniverseObjectType::OBJ_SHIP};
    IncapacitationsEvent planet_incapacitations{UniverseObjectType::OBJ_PLANET};
    IncapacitationsEvent other_incapacitations;     // mostly for backwards compatability, for which type of incapcitated object is unknown

private:
    template <typename Archive>
    friend void serialize(Archive&, BoutEvent&, unsigned int const);
};


#endif
