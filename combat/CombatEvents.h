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

/// Generated when a new bout begins in the battle
struct FO_COMMON_API BoutBeginEvent final : public CombatEvent {
    [[nodiscard]] constexpr BoutBeginEvent() noexcept = default;
    [[nodiscard]] constexpr explicit BoutBeginEvent(int bout_) noexcept :
        bout(bout_)
    {}

    [[nodiscard]] std::string DebugString(const ScriptingContext& context) const override;
    [[nodiscard]] std::string CombatLogDescription(int viewing_empire_id, const ScriptingContext& context) const override;

    int bout = 0;
};

/** BoutEvent describes all the events that happen in one bout of combat.
   It may have some sub-events and the sub-events will be ordered.*/
struct FO_COMMON_API BoutEvent : public CombatEvent {
    typedef std::shared_ptr<BoutEvent> BoutEventPtr;

    [[nodiscard]] CONSTEXPR_VEC BoutEvent() noexcept(CombatEventDetail::nxcepv) = default;
    [[nodiscard]] CONSTEXPR_VEC explicit BoutEvent(int bout_) noexcept(CombatEventDetail::nxcepv) :
        bout(bout_)
    {}

    void AddEvent(CombatEventPtr event) { events.push_back(std::move(event)); }

    [[nodiscard]] std::string DebugString(const ScriptingContext& context) const override;
    [[nodiscard]] std::string CombatLogDescription(int viewing_empire_id, const ScriptingContext& context) const override;
    [[nodiscard]] std::vector<ConstCombatEventPtr> SubEvents(int viewing_empire_id) const override;

    [[nodiscard]] bool AreSubEventsEmpty(int) const noexcept override { return events.empty(); }

    [[nodiscard]] bool FlattenSubEvents() const noexcept override { return true; }

private:
    int bout = 0;

    std::vector<CombatEventPtr> events;

    template <typename Archive>
    friend void serialize(Archive&, BoutEvent&, unsigned int const);
};

/** SimultaneousEvents describes a set of simultaneous events in one bout of combat.
   It may have some sub-events and the sub-events will be shown in
   viewing empire first followed by ALL_EMPIRES and then other empires.*/
struct FO_COMMON_API SimultaneousEvents : public CombatEvent {
    [[nodiscard]] CONSTEXPR_VEC SimultaneousEvents() noexcept(CombatEventDetail::nxcepv) = default;

    void AddEvent(CombatEventPtr event);

    [[nodiscard]] std::string DebugString(const ScriptingContext&) const override;
    [[nodiscard]] std::string CombatLogDescription(int, const ScriptingContext&)
        const noexcept(CombatEventDetail::nxstr) override { return {}; }
    [[nodiscard]] std::vector<ConstCombatEventPtr> SubEvents(int viewing_empire_id) const override;

    [[nodiscard]] bool AreSubEventsEmpty(int) const noexcept override { return events.empty(); }

    [[nodiscard]] virtual bool FlattenSubEvents() const override
    { return true; }

protected:
    std::vector<CombatEventPtr> events;

    template <typename Archive>
    friend void serialize(Archive&, SimultaneousEvents&, unsigned int const);
};


typedef SimultaneousEvents AttacksEvent;
typedef std::shared_ptr<AttacksEvent> AttacksEventPtr;

typedef SimultaneousEvents IncapacitationsEvent;
typedef std::shared_ptr<IncapacitationsEvent> IncapacitationsEventPtr;

typedef SimultaneousEvents FighterLaunchesEvent;
typedef std::shared_ptr<FighterLaunchesEvent> FighterLaunchesEventPtr;


/** InitialStealthEvent describes the initially stealthy combatants.
    Note:  Because it is initialized with the unfiltered stealth information it
    contains information not availble to all empires. */
struct FO_COMMON_API InitialStealthEvent : public CombatEvent {
    typedef std::map<int, std::map<int, Visibility>> EmpireToObjectVisibilityMap;

    [[nodiscard]] InitialStealthEvent() = default;
    [[nodiscard]] explicit InitialStealthEvent(EmpireToObjectVisibilityMap x) :
        empire_to_object_visibility(std::move(x))
    {}

    [[nodiscard]] std::string DebugString(const ScriptingContext& context) const override;
    [[nodiscard]] std::string CombatLogDescription(int viewing_empire_id, const ScriptingContext& context) const override;

private:
    EmpireToObjectVisibilityMap empire_to_object_visibility;

    template <typename Archive>
    friend void serialize(Archive&, InitialStealthEvent&, unsigned int const);
};

/**StealthChangeEvent describes changes in the visibility of objects during combat.
 At this time always decloaking.*/
struct FO_COMMON_API StealthChangeEvent : public CombatEvent {
    [[nodiscard]] StealthChangeEvent() = default;
    [[nodiscard]] explicit StealthChangeEvent(int bout_) :
        bout(bout_)
    {}

    [[nodiscard]] std::string DebugString(const ScriptingContext& context) const override;
    [[nodiscard]] std::string CombatLogDescription(int viewing_empire_id, const ScriptingContext& context) const override;
    [[nodiscard]] std::vector<ConstCombatEventPtr> SubEvents(int viewing_empire_id) const override;
    [[nodiscard]] bool AreSubEventsEmpty(int) const noexcept override { return events.empty(); }

    void AddEvent(int attacker_id_, int target_id_, int attacker_empire_,
                  int target_empire_, Visibility new_visibility_);
    void AddEvent(int launcher_id_, int launcher_empire_id_,
                  int observer_empire_id_, Visibility new_visibility_);

    struct StealthChangeEventDetail;
    typedef std::shared_ptr<StealthChangeEventDetail> StealthChangeEventDetailPtr;
    typedef std::shared_ptr<const StealthChangeEventDetail> ConstStealthChangeEventDetailPtr;
    struct StealthChangeEventDetail : public CombatEvent {
        [[nodiscard]] constexpr StealthChangeEventDetail() noexcept = default;
        [[nodiscard]] constexpr StealthChangeEventDetail(int attacker_id_, int target_id_, int attacker_empire_,
                                                         int target_empire_, Visibility new_visibility_) noexcept :
            attacker_id(attacker_id_),
            target_id(target_id_),
            attacker_empire_id(attacker_empire_),
            target_empire_id(target_empire_),
            visibility(new_visibility_)
        {}
        [[nodiscard]] constexpr StealthChangeEventDetail(int laucher_id_, int laucher_empire_, int observer_empire_,
                                                         Visibility new_visibility_) noexcept :
            attacker_id(laucher_id_),
            attacker_empire_id(laucher_empire_),
            target_empire_id(observer_empire_),
            visibility(new_visibility_),
            is_fighter_launch(true)
        {}

        [[nodiscard]] std::string DebugString(const ScriptingContext& context) const override;
        [[nodiscard]] std::string CombatLogDescription(int viewing_empire_id, const ScriptingContext& context) const override;

        int attacker_id = INVALID_OBJECT_ID;
        int target_id = INVALID_OBJECT_ID;
        int attacker_empire_id = ALL_EMPIRES;
        int target_empire_id = ALL_EMPIRES;
        Visibility visibility = Visibility::VIS_NO_VISIBILITY;
        bool is_fighter_launch = false;
    };

private:
    int bout = -1;
    std::map<int, std::vector<StealthChangeEventDetailPtr>> events;

    template <typename Archive>
    friend void serialize(Archive&, StealthChangeEvent&, unsigned int const);
};

/// An event that describes a single attack by one object or fighter against
/// another object or fighter
struct FO_COMMON_API WeaponFireEvent : public CombatEvent {
    typedef std::shared_ptr<WeaponFireEvent> WeaponFireEventPtr;
    typedef std::shared_ptr<const WeaponFireEvent> ConstWeaponFireEventPtr;

    [[nodiscard]] CONSTEXPR_STRING WeaponFireEvent() noexcept(CombatEventDetail::nxstr) = default;

    /** WeaponFireEvent encodes a single attack in \p bout, \p round by \p
        attacker_id owned by \p attacker_owner_id on \p target_id with \p
        weapon_name of \p power against \p shield doing \p damage.

        If \p shield is negative that implies the weapon is shield piercing.

        The use of tuple in the constructor is to keep the number of parameters below 10 which
        is the maximum that some compilers that emulate variadic templates support. */
    [[nodiscard]] CONSTEXPR_STRING WeaponFireEvent(int bout_, int round_, int attacker_id_, int target_id_,
                                                   std::string weapon_name_,
                                                   const std::tuple<float, float, float>& power_shield_damage,
                                                   int attacker_owner_id_, int target_owner_id_)
        noexcept(CombatEventDetail::nxstrmove) :
        bout(bout_),
        round(round_),
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
    [[nodiscard]] bool AreDetailsEmpty(int) const noexcept override { return false; }
    [[nodiscard]] boost::optional<int> PrincipalFaction(int viewing_empire_id) const noexcept override { return attacker_id; }

    int bout = -1;
    int round = -1;
    int attacker_id = INVALID_OBJECT_ID;
    int target_id = INVALID_OBJECT_ID;
    std::string weapon_name;
    float power = 0.0f;
    float shield = 0.0f;
    float damage = 0.0f;
    int attacker_owner_id = ALL_EMPIRES;
    int target_owner_id = ALL_EMPIRES;
};

/// Created when an object becomes unable to fight anymore,
/// eg. a ship is destroyed or a planet loses all defence
struct FO_COMMON_API IncapacitationEvent : public CombatEvent {
    [[nodiscard]] constexpr IncapacitationEvent() noexcept = default;
    [[nodiscard]] constexpr IncapacitationEvent(int bout_, int object_id_, int object_owner_id_) noexcept :
        bout(bout_),
        object_id(object_id_),
        object_owner_id(object_owner_id_)
    {}

    [[nodiscard]] std::string DebugString(const ScriptingContext& context) const override;
    [[nodiscard]] std::string CombatLogDescription(int viewing_empire_id, const ScriptingContext& context) const override;
    [[nodiscard]] boost::optional<int> PrincipalFaction(int viewing_empire_id) const override;

    int bout = -1;
    int object_id = INVALID_OBJECT_ID;
    int object_owner_id = ALL_EMPIRES;
};


/** FightersAttackFightersEvent aggregates all the fighter on fighter combat for one bout.*/
struct FO_COMMON_API FightersAttackFightersEvent : public CombatEvent {
    [[nodiscard]] FightersAttackFightersEvent() = default;
    [[nodiscard]] explicit FightersAttackFightersEvent(int bout_) :
        bout(bout_)
    {}

    [[nodiscard]] std::string DebugString(const ScriptingContext& context) const override;
    [[nodiscard]] std::string CombatLogDescription(int viewing_empire_id, const ScriptingContext& context) const override;
    void AddEvent(int attacker_empire_, int target_empire_);

private:
    int bout = 0;

    // Store the number of each of the identical fighter combat events.
    std::map<std::pair<int, int>, unsigned int> events;

    template <typename Archive>
    friend void serialize(Archive&, FightersAttackFightersEvent&, unsigned int const);
};

/// Created when an fighter is launched
struct FO_COMMON_API FighterLaunchEvent : public CombatEvent {
    typedef std::shared_ptr<FighterLaunchEvent> FighterLaunchEventPtr;

    [[nodiscard]] constexpr FighterLaunchEvent() noexcept = default;
    [[nodiscard]] constexpr FighterLaunchEvent(int bout_, int launched_from_id_,
                                               int fighter_owner_empire_id_, int number_launched_) noexcept :
        bout(bout_),
        fighter_owner_empire_id(fighter_owner_empire_id_),
        launched_from_id(launched_from_id_),
        number_launched(number_launched_)
    {}

    [[nodiscard]] std::string DebugString(const ScriptingContext& context) const override;
    [[nodiscard]] std::string CombatLogDescription(int viewing_empire_id, const ScriptingContext& context) const override;
    [[nodiscard]] boost::optional<int> PrincipalFaction(int viewing_empire_id) const override;

    int bout = 0;
    int fighter_owner_empire_id = ALL_EMPIRES;
    int launched_from_id = INVALID_OBJECT_ID;
    int number_launched = 0;
};

/** FightersDestroyedEvent aggregates all the fighters destroyed during one combat bout.*/
struct FO_COMMON_API FightersDestroyedEvent : public CombatEvent {
    [[nodiscard]] FightersDestroyedEvent() = default;
    [[nodiscard]] explicit FightersDestroyedEvent(int bout_) :
        bout(bout_)
    {}

    [[nodiscard]] std::string DebugString(const ScriptingContext& context) const override;
    [[nodiscard]] std::string CombatLogDescription(int viewing_empire_id, const ScriptingContext& context) const override;
    void AddEvent(int target_empire_);

private:
    int bout = 0;

    // Store the number of each of the identical fighter combat events.
    std::map<int, unsigned int> events;

    template <typename Archive>
    friend void serialize(Archive&, FightersDestroyedEvent&, unsigned int const);
};

/** WeaponsPlatformEvent describes a ship or planet with zero or more weapons firing its weapons in combat.
   It may have some WeaponFire sub-events.*/
struct FO_COMMON_API WeaponsPlatformEvent : public CombatEvent {
    typedef std::shared_ptr<WeaponsPlatformEvent> WeaponsPlatformEventPtr;
    typedef std::shared_ptr<const WeaponsPlatformEvent> ConstWeaponsPlatformEventPtr;

    [[nodiscard]] WeaponsPlatformEvent() = default;
    [[nodiscard]] WeaponsPlatformEvent(int bout_, int attacker_id_, int attacker_owner_id_) :
        bout(bout_),
        attacker_id(attacker_id_),
        attacker_owner_id(attacker_owner_id_)
    {}

    void AddEvent(int round, int target_id, int target_owner_id_, const std::string& weapon_name_,
                  float power_, float shield_, float damage_);

    [[nodiscard]] std::string DebugString(const ScriptingContext& context) const override;
    [[nodiscard]] std::string CombatLogDescription(int viewing_empire_id, const ScriptingContext& context) const override;
    [[nodiscard]] std::vector<ConstCombatEventPtr> SubEvents(int viewing_empire_id) const override;
    [[nodiscard]] bool AreSubEventsEmpty(int) const noexcept override { return events.empty(); }
    [[nodiscard]] boost::optional<int> PrincipalFaction(int viewing_empire_id) const override;

    int bout = 0;
    int attacker_id = INVALID_OBJECT_ID;
    int attacker_owner_id = ALL_EMPIRES;

private:
    std::map<int, std::vector<WeaponFireEvent::WeaponFireEventPtr>> events;

    template <typename Archive>
    friend void serialize(Archive&, WeaponsPlatformEvent&, unsigned int const);
};


#endif
