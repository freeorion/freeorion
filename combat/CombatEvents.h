#ifndef COMBATEVENTS_H
#define COMBATEVENTS_H

#include <set>
#include <boost/tuple/tuple.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/export.hpp>

#include "../util/Export.h"
#include "../universe/Enums.h"

#include "CombatEvent.h"

/** \file
 * Contains all combat event implementation declarations.
 */

/// Generated when a new bout begins in the battle
struct FO_COMMON_API BoutBeginEvent : public CombatEvent {
    BoutBeginEvent();
    BoutBeginEvent(int bout);
    virtual ~BoutBeginEvent() {}

    virtual std::string DebugString() const;
    virtual std::string CombatLogDescription(int viewing_empire_id) const;

    int bout;

private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** BoutEvent describes all the events that happen in one bout of combat.
   It may have some sub-events and the sub-events will be ordered.*/
struct FO_COMMON_API BoutEvent : public CombatEvent {
    typedef boost::shared_ptr<BoutEvent> BoutEventPtr;

    BoutEvent();
    BoutEvent(int bout);

    virtual ~BoutEvent() {}

    void AddEvent(CombatEventPtr & event);

    virtual std::string DebugString() const;
    virtual std::string CombatLogDescription(int viewing_empire_id) const;
    virtual std::vector<ConstCombatEventPtr> SubEvents(int viewing_empire_id) const;
    virtual bool AreSubEventsEmpty(int viewing_empire_id) const
    { return events.empty();}
    virtual bool AreSubEventsOrdered() const
    { return true; }
    virtual bool FlattenSubEvents() const
    { return true; }

private:
    int bout;

    std::vector<CombatEventPtr> events;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** SimultaneousEvents describes a set of simultaneous events in one bout of combat.
   It may have some sub-events and the sub-events will be shown in
   viewing empire first followed by ALL_EMPIRES and then other empires.*/
struct FO_COMMON_API SimultaneousEvents : public CombatEvent {
    typedef boost::shared_ptr<SimultaneousEvents> SimultaneousEventsPtr;

    SimultaneousEvents();

    virtual ~SimultaneousEvents() {}

    void AddEvent(CombatEventPtr &event);

    virtual std::string DebugString() const;
    virtual std::string CombatLogDescription(int viewing_empire_id) const;
    virtual std::vector<ConstCombatEventPtr> SubEvents(int viewing_empire_id) const;
    virtual bool AreSubEventsEmpty(int viewing_empire_id) const
    { return events.empty();}
    virtual bool FlattenSubEvents() const
    { return true; }

protected:
    std::vector<CombatEventPtr> events;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};


typedef SimultaneousEvents AttacksEvent;
typedef boost::shared_ptr<AttacksEvent> AttacksEventPtr;

typedef SimultaneousEvents IncapacitationsEvent;
typedef boost::shared_ptr<IncapacitationsEvent> IncapacitationsEventPtr;

typedef SimultaneousEvents FighterLaunchesEvent;
typedef boost::shared_ptr<FighterLaunchesEvent> FighterLaunchesEventPtr;


/** InitialStealthEvent describes the initially stealthy combatants.
    Note:  Because it is initialized with the unfiltered stealth information it
    contains information not availble to all empires. */
struct FO_COMMON_API InitialStealthEvent : public CombatEvent {

    /// a map of attacker empire id -> defender empire id -> set of (attacker ids, visibility)
    typedef std::map<int, std::map< int, std::set< std::pair<int, Visibility> > > > StealthInvisbleMap;

    InitialStealthEvent();
    InitialStealthEvent(const StealthInvisbleMap &);

    virtual ~InitialStealthEvent() {}

    virtual std::string DebugString() const;
    virtual std::string CombatLogDescription(int viewing_empire_id) const;

private:

    StealthInvisbleMap target_empire_id_to_invisble_obj_id;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/**StealthChangeEvent describes changes in the visibility of objects during combat.
 At this time always decloaking.*/
struct FO_COMMON_API StealthChangeEvent : public CombatEvent {
    StealthChangeEvent();
    StealthChangeEvent(int bout);

    virtual ~StealthChangeEvent() {}

    void AddEvent(int attacker_id_, int target_id_, int attacker_empire_, int target_empire_, Visibility new_visibility_);

    virtual std::string DebugString() const;
    virtual std::string CombatLogDescription(int viewing_empire_id) const;
    virtual std::vector<ConstCombatEventPtr> SubEvents(int viewing_empire_id) const;
    virtual bool AreSubEventsEmpty(int viewing_empire_id) const;

    struct StealthChangeEventDetail;
    typedef boost::shared_ptr<StealthChangeEventDetail> StealthChangeEventDetailPtr;
    typedef boost::shared_ptr<const StealthChangeEventDetail> ConstStealthChangeEventDetailPtr;
    struct StealthChangeEventDetail : public CombatEvent {
        int attacker_id;
        int target_id;
        int attacker_empire_id;
        int target_empire_id;
        Visibility visibility;
        StealthChangeEventDetail();
        StealthChangeEventDetail(int attacker_id_, int target_id_, int attacker_empire_, int target_empire_, Visibility new_visibility_);

        virtual std::string DebugString() const;
        virtual std::string CombatLogDescription(int viewing_empire_id) const;

        friend class boost::serialization::access;
        template <class Archive>
        void serialize(Archive& ar, const unsigned int version);
    };

private:
    int     bout;

    std::map<int, std::vector<StealthChangeEventDetailPtr> > events;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/// An event that describes a single attack by one object or fighter against another object or fighter
struct FO_COMMON_API WeaponFireEvent : public CombatEvent {
    typedef boost::shared_ptr<WeaponFireEvent> WeaponFireEventPtr;
    typedef boost::shared_ptr<const WeaponFireEvent> ConstWeaponFireEventPtr;

    WeaponFireEvent();

    /** WeaponFireEvent encodes a single attack in \p bout, \p round by \p
        attacker_id owned by \p attacker_owner_id on \p target_id with \p
        weapon_name of \p power against \p shield doing \p damage.

        If \p shield is negative that implies the weapon is shield piercing.

        The use of tuple in the constructor is to keep the number of parameters below 10 which
        is the maximum that some compilers that emulate variadic templates support.
     */
    WeaponFireEvent(int bout, int round, int attacker_id, int target_id, const std::string &weapon_name,
                    const boost::tuple<float, float, float> &power_shield_damage,
                    int attacker_owner_id_, int target_owner_id_);

    virtual ~WeaponFireEvent() {}

    virtual std::string DebugString() const;
    virtual std::string CombatLogDescription(int viewing_empire_id) const;
    virtual std::string CombatLogDetails(int viewing_empire_id) const;
    virtual bool AreDetailsEmpty(int viewing_empire_id) const
    { return false; }
    virtual boost::optional<int> PrincipalFaction(int viewing_empire_id) const;

    int     bout;
    int     round;
    int     attacker_id;
    int     target_id;
    std::string weapon_name;
    float   power;
    float   shield;
    float   damage;
    int     attacker_owner_id;
    int     target_owner_id;

private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/// Created when an object becomes unable to fight anymore,
/// eg. a ship is destroyed or a planet loses all defence
struct FO_COMMON_API IncapacitationEvent : public CombatEvent {
    IncapacitationEvent();
    IncapacitationEvent(int bout_, int object_id_, int object_owner_id_);
    virtual ~IncapacitationEvent() {}

    virtual std::string DebugString() const;
    virtual std::string CombatLogDescription(int viewing_empire_id) const;
    virtual boost::optional<int> PrincipalFaction(int viewing_empire_id) const;

    int bout;
    int object_id;
    int object_owner_id;

private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/// Created when an fighter is destroyed
struct FO_COMMON_API FighterAttackedEvent : public CombatEvent {
    typedef boost::shared_ptr<FighterAttackedEvent> FighterAttackedEventPtr;

    FighterAttackedEvent();
    FighterAttackedEvent(int bout_, int round_, int attacked_by_object_id_, int attacker_owner_empire_id_, int attacked_owner_id_);
    virtual ~FighterAttackedEvent() {}

    virtual std::string DebugString() const;
    virtual std::string CombatLogDescription(int viewing_empire_id) const;
    virtual boost::optional<int> PrincipalFaction(int viewing_empire_id) const;

    int bout;
    int round;
    int attacker_owner_empire_id;   // may be ALL_EMPIRES if attacking fighter was owned by no empire
    int attacked_by_object_id;     // may be INVALID_OBJECT_ID if destroyed by another fighter
    int attacked_owner_id;         // may be ALL_EMPIRES if destroyed fighter was owned by no empire

private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/// Created when an fighter is launched
struct FO_COMMON_API FighterLaunchEvent : public CombatEvent {
    typedef boost::shared_ptr<FighterLaunchEvent> FighterLaunchEventPtr;

    FighterLaunchEvent();
    FighterLaunchEvent(int bout_, int launched_from_id_, int fighter_owner_empire_id_, int number_launched_);
    virtual ~FighterLaunchEvent() {}

    virtual std::string DebugString() const;
    virtual std::string CombatLogDescription(int viewing_empire_id) const;
    virtual boost::optional<int> PrincipalFaction(int viewing_empire_id) const;

    int bout;
    int fighter_owner_empire_id;    // may be ALL_EMPIRE if fighter was owned by no empire
    int launched_from_id;
    int number_launched;

private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** WeaponsPlatformEvent describes a ship or planet with zero or more weapons firing its weapons in combat.
   It may have some WeaponFire sub-events.*/
struct FO_COMMON_API WeaponsPlatformEvent : public CombatEvent {
    typedef boost::shared_ptr<WeaponsPlatformEvent> WeaponsPlatformEventPtr;
    typedef boost::shared_ptr<const WeaponsPlatformEvent> ConstWeaponsPlatformEventPtr;

    WeaponsPlatformEvent();
    WeaponsPlatformEvent(int bout, int attacker_id, int attacker_owner_id_);

    virtual ~WeaponsPlatformEvent() {}

    void AddEvent(int round, int target_id, int target_owner_id_, std::string const & weapon_name_,
                  float power_, float shield_, float damage_);

    virtual std::string DebugString() const;
    virtual std::string CombatLogDescription(int viewing_empire_id) const;
    virtual std::vector<ConstCombatEventPtr> SubEvents(int viewing_empire_id) const;
    virtual bool AreSubEventsEmpty(int viewing_empire_id) const;
    virtual boost::optional<int> PrincipalFaction(int viewing_empire_id) const;

    int bout;
    int attacker_id;
    int attacker_owner_id;

private:
    std::map<int, std::vector<WeaponFireEvent::WeaponFireEventPtr> > events;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

#endif // COMBATEVENT_H
