#ifndef COMBATEVENTS_H
#define COMBATEVENTS_H


#include <map>
#include <memory>
#include <set>

#include <tuple>

#include "../util/Export.h"
#include "../universe/ConstantsFwd.h"
#include "../universe/EnumsFwd.h"

#include "CombatEvent.h"


/// Generated when a new bout begins in the battle
struct FO_COMMON_API BoutBeginEvent : public CombatEvent {
    BoutBeginEvent() = default;
    explicit BoutBeginEvent(int bout);

    [[nodiscard]] std::string DebugString(const ScriptingContext& context) const override;
    [[nodiscard]] std::string CombatLogDescription(int viewing_empire_id, const ScriptingContext& context) const override;

    int bout = 0;
};

/** BoutEvent describes all the events that happen in one bout of combat.
   It may have some sub-events and the sub-events will be ordered.*/
struct FO_COMMON_API BoutEvent : public CombatEvent {
    typedef std::shared_ptr<BoutEvent> BoutEventPtr;

    BoutEvent() = default;
    explicit BoutEvent(int bout);

    void AddEvent(CombatEventPtr event);

    [[nodiscard]] std::string DebugString(const ScriptingContext& context) const override;
    [[nodiscard]] std::string CombatLogDescription(int viewing_empire_id, const ScriptingContext& context) const override;
    [[nodiscard]] std::vector<ConstCombatEventPtr> SubEvents(int viewing_empire_id) const override;

    [[nodiscard]] bool AreSubEventsEmpty(int viewing_empire_id) const override
    { return events.empty(); }

    [[nodiscard]] bool FlattenSubEvents() const override
    { return true; }

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
    typedef std::shared_ptr<SimultaneousEvents> SimultaneousEventsPtr;

    SimultaneousEvents() = default;

    void AddEvent(CombatEventPtr event);

    [[nodiscard]] FO_COMMON_API std::string DebugString(const ScriptingContext& context) const override
    { return "SimultaneousEvents has " + std::to_string(events.size()) + " events"; } // no idea why, but the linker refuses to find this function if defined in the .cpp
    [[nodiscard]] FO_COMMON_API std::string CombatLogDescription(int viewing_empire_id, const ScriptingContext& context) const override
    { return ""; } // no idea why, but the linker refuses to find this function if defined in the .cpp
    [[nodiscard]] std::vector<ConstCombatEventPtr> SubEvents(int viewing_empire_id) const override;

    [[nodiscard]] bool AreSubEventsEmpty(int viewing_empire_id) const override
    { return events.empty(); }

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

    InitialStealthEvent() = default;
    explicit InitialStealthEvent(const EmpireToObjectVisibilityMap& x);

    [[nodiscard]] std::string DebugString(const ScriptingContext& context) const override;
    [[nodiscard]] std::string CombatLogDescription(int viewing_empire_id, const ScriptingContext& context) const override;

private:
    EmpireToObjectVisibilityMap empire_to_object_visibility;// filled by AutoresolveInfo::ReportInvisibleObjects

    template <typename Archive>
    friend void serialize(Archive&, InitialStealthEvent&, unsigned int const);
};

/**StealthChangeEvent describes changes in the visibility of objects during combat.
 At this time always decloaking.*/
struct FO_COMMON_API StealthChangeEvent : public CombatEvent {
    StealthChangeEvent() = default;
    explicit StealthChangeEvent(int bout);

    [[nodiscard]] std::string DebugString(const ScriptingContext& context) const override;
    [[nodiscard]] std::string CombatLogDescription(int viewing_empire_id, const ScriptingContext& context) const override;
    [[nodiscard]] std::vector<ConstCombatEventPtr> SubEvents(int viewing_empire_id) const override;
    [[nodiscard]] bool AreSubEventsEmpty(int viewing_empire_id) const override;
    void AddEvent(int attacker_id_, int target_id_, int attacker_empire_,
                  int target_empire_, Visibility new_visibility_);

    struct StealthChangeEventDetail;
    typedef std::shared_ptr<StealthChangeEventDetail> StealthChangeEventDetailPtr;
    typedef std::shared_ptr<const StealthChangeEventDetail> ConstStealthChangeEventDetailPtr;
    struct StealthChangeEventDetail : public CombatEvent {
        StealthChangeEventDetail();
        StealthChangeEventDetail(int attacker_id_, int target_id_, int attacker_empire_,
                                 int target_empire_, Visibility new_visibility_);

        [[nodiscard]] std::string DebugString(const ScriptingContext& context) const override;
        [[nodiscard]] std::string CombatLogDescription(int viewing_empire_id, const ScriptingContext& context) const override;

        int attacker_id = INVALID_OBJECT_ID;
        int target_id = INVALID_OBJECT_ID;
        int attacker_empire_id = ALL_EMPIRES;
        int target_empire_id = ALL_EMPIRES;
        Visibility visibility;
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

    WeaponFireEvent() = default;

    /** WeaponFireEvent encodes a single attack in \p bout, \p round by \p
        attacker_id owned by \p attacker_owner_id on \p target_id with \p
        weapon_name of \p power against \p shield doing \p damage.

        If \p shield is negative that implies the weapon is shield piercing.

        The use of tuple in the constructor is to keep the number of parameters below 10 which
        is the maximum that some compilers that emulate variadic templates support. */
    WeaponFireEvent(int bout, int round, int attacker_id, int target_id, std::string weapon_name_,
                    const std::tuple<float, float, float>& power_shield_damage,
                    int attacker_owner_id_, int target_owner_id_);
    [[nodiscard]] std::string DebugString(const ScriptingContext& context) const override;
    [[nodiscard]] std::string CombatLogDescription(int viewing_empire_id, const ScriptingContext& context) const override;
    [[nodiscard]] std::string CombatLogDetails(int viewing_empire_id) const override;
    [[nodiscard]] bool AreDetailsEmpty(int viewing_empire_id) const override { return false; }
    [[nodiscard]] boost::optional<int> PrincipalFaction(int viewing_empire_id) const override;

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
    explicit IncapacitationEvent();
    IncapacitationEvent(int bout_, int object_id_, int object_owner_id_);
    [[nodiscard]] std::string DebugString(const ScriptingContext& context) const override;
    [[nodiscard]] std::string CombatLogDescription(int viewing_empire_id, const ScriptingContext& context) const override;
    [[nodiscard]] boost::optional<int> PrincipalFaction(int viewing_empire_id) const override;

    int bout = 0;
    int object_id = INVALID_OBJECT_ID;
    int object_owner_id = ALL_EMPIRES;
};


/** FightersAttackFightersEvent aggregates all the fighter on fighter combat for one bout.*/
struct FO_COMMON_API FightersAttackFightersEvent : public CombatEvent {
    FightersAttackFightersEvent() = default;
    explicit FightersAttackFightersEvent(int bout);
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

    FighterLaunchEvent() = default;
    FighterLaunchEvent(int bout_, int launched_from_id_, int fighter_owner_empire_id_, int number_launched_);
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
    FightersDestroyedEvent() = default;
    explicit FightersDestroyedEvent(int bout);
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

    WeaponsPlatformEvent() = default;
    WeaponsPlatformEvent(int bout, int attacker_id, int attacker_owner_id_);

    void AddEvent(int round, int target_id, int target_owner_id_, const std::string& weapon_name_,
                  float power_, float shield_, float damage_);

    [[nodiscard]] std::string DebugString(const ScriptingContext& context) const override;
    [[nodiscard]] std::string CombatLogDescription(int viewing_empire_id, const ScriptingContext& context) const override;
    [[nodiscard]] std::vector<ConstCombatEventPtr> SubEvents(int viewing_empire_id) const override;
    [[nodiscard]] bool AreSubEventsEmpty(int viewing_empire_id) const override;
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
