#ifndef _CombatSystem_h_
#define _CombatSystem_h_

#include "../universe/Universe.h"
#include "../util/AppInterface.h"
#include "CombatEvent.h"

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/version.hpp>

/** Contains information about the state of a combat before or after the combat
  * occurs. */
struct CombatInfo {
public:
    /** \name Structors */ //@{
    CombatInfo() = default;
    CombatInfo(int system_id_, int turn_);  ///< ctor taking system id where combat occurs and game turn on which combat occurs
    //@}

    /** \name Accessors */ //@{
    /** Returns System object in this CombatInfo's objects if one exists with
        id system_id. */
    std::shared_ptr<const System> GetSystem() const;
    //@}

    /** \name Mutators */ //@{
    /** Returns System object in this CombatInfo's objects if one exists with
        id system_id. */
    std::shared_ptr<System> GetSystem();

    int                                 turn = INVALID_GAME_TURN;       ///< main game turn
    int                                 system_id = INVALID_OBJECT_ID;  ///< ID of system where combat is occurring (could be INVALID_OBJECT_ID ?)
    std::set<int>                       empire_ids;                     ///< IDs of empires involved in combat
    ObjectMap                           objects;                        ///< actual state of objects relevant to combat
    std::set<int>                       damaged_object_ids;             ///< ids of objects damaged during this battle
    std::set<int>                       destroyed_object_ids;           ///< ids of objects destroyed during this battle
    std::map<int, std::set<int>>        destroyed_object_knowers;       ///< indexed by empire ID, the set of ids of objects the empire knows were destroyed during the combat
    Universe::EmpireObjectVisibilityMap empire_object_visibility;       ///< indexed by empire id and object id, the visibility level the empire has of each object.  may be increased during battle
    std::vector<CombatEventPtr>         combat_events;                  ///< list of combat attack events that occur in combat

    float   GetMonsterDetection() const;

private:
    void    GetEmpireIdsToSerialize(             std::set<int>&                         filtered_empire_ids,                int encoding_empire) const;
    void    GetObjectsToSerialize(               ObjectMap&                             filtered_objects,                   int encoding_empire) const;
    void    GetDamagedObjectsToSerialize(        std::set<int>&                         filtered_damaged_objects,           int encoding_empire) const;
    void    GetDestroyedObjectsToSerialize(      std::set<int>&                         filtered_destroyed_objects,         int encoding_empire) const;
    void    GetDestroyedObjectKnowersToSerialize(std::map<int, std::set<int>>&          filtered_destroyed_object_knowers,  int encoding_empire) const;
    void    GetEmpireObjectVisibilityToSerialize(Universe::EmpireObjectVisibilityMap&   filtered_empire_object_visibility,  int encoding_empire) const;
    void    GetCombatEventsToSerialize(          std::vector<CombatEventPtr>&           filtered_combat_events,             int encoding_empire) const;

    void    InitializeObjectVisibility();

    friend class boost::serialization::access;
    template<class Archive>
    void save(Archive & ar, const unsigned int version) const;
    template<class Archive>
    void load(Archive & ar, const unsigned int version);
    BOOST_SERIALIZATION_SPLIT_MEMBER()
};

/** Auto-resolves a battle. */
void AutoResolveCombat(CombatInfo& combat_info);

template <class Archive>
void CombatInfo::save(Archive & ar, const unsigned int version) const
{
    std::set<int>                       filtered_empire_ids;
    ObjectMap                           filtered_objects;
    std::set<int>                       filtered_damaged_object_ids;
    std::set<int>                       filtered_destroyed_object_ids;
    std::map<int, std::set<int>>        filtered_destroyed_object_knowers;
    Universe::EmpireObjectVisibilityMap filtered_empire_object_visibility;
    std::vector<CombatEventPtr>         filtered_combat_events;

    GetEmpireIdsToSerialize(                filtered_empire_ids,                GetUniverse().EncodingEmpire());
    GetObjectsToSerialize(                  filtered_objects,                   GetUniverse().EncodingEmpire());
    GetDamagedObjectsToSerialize(           filtered_damaged_object_ids,        GetUniverse().EncodingEmpire());
    GetDestroyedObjectsToSerialize(         filtered_destroyed_object_ids,      GetUniverse().EncodingEmpire());
    GetDestroyedObjectKnowersToSerialize(   filtered_destroyed_object_knowers,  GetUniverse().EncodingEmpire());
    GetEmpireObjectVisibilityToSerialize(   filtered_empire_object_visibility,  GetUniverse().EncodingEmpire());
    GetCombatEventsToSerialize(             filtered_combat_events,             GetUniverse().EncodingEmpire());

    ar  & BOOST_SERIALIZATION_NVP(turn)
        & BOOST_SERIALIZATION_NVP(system_id)
        & BOOST_SERIALIZATION_NVP(filtered_empire_ids)
        & BOOST_SERIALIZATION_NVP(filtered_objects)
        & BOOST_SERIALIZATION_NVP(filtered_damaged_object_ids)
        & BOOST_SERIALIZATION_NVP(filtered_destroyed_object_ids)
        & BOOST_SERIALIZATION_NVP(filtered_destroyed_object_knowers)
        & BOOST_SERIALIZATION_NVP(filtered_empire_object_visibility)
        & BOOST_SERIALIZATION_NVP(filtered_combat_events);
}

template <class Archive>
void CombatInfo::load(Archive & ar, const unsigned int version)
{
    std::set<int>                       filtered_empire_ids;
    ObjectMap                           filtered_objects;
    std::set<int>                       filtered_damaged_object_ids;
    std::set<int>                       filtered_destroyed_object_ids;
    std::map<int, std::set<int>>        filtered_destroyed_object_knowers;
    Universe::EmpireObjectVisibilityMap filtered_empire_object_visibility;
    std::vector<CombatEventPtr>         filtered_combat_events;

    ar  & BOOST_SERIALIZATION_NVP(turn)
        & BOOST_SERIALIZATION_NVP(system_id)
        & BOOST_SERIALIZATION_NVP(filtered_empire_ids)
        & BOOST_SERIALIZATION_NVP(filtered_objects)
        & BOOST_SERIALIZATION_NVP(filtered_damaged_object_ids)
        & BOOST_SERIALIZATION_NVP(filtered_destroyed_object_ids)
        & BOOST_SERIALIZATION_NVP(filtered_destroyed_object_knowers)
        & BOOST_SERIALIZATION_NVP(filtered_empire_object_visibility)
        & BOOST_SERIALIZATION_NVP(filtered_combat_events);

    empire_ids.swap(              filtered_empire_ids);
    objects.swap(                 filtered_objects);
    damaged_object_ids.swap(      filtered_damaged_object_ids);
    destroyed_object_ids.swap(    filtered_destroyed_object_ids);
    destroyed_object_knowers.swap(filtered_destroyed_object_knowers);
    empire_object_visibility.swap(filtered_empire_object_visibility);
    combat_events.swap(           filtered_combat_events);
}

#endif // _CombatSystem_h_
