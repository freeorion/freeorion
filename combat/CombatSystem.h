// -*- C++ -*-
#ifndef _CombatSystem_h_
#define _CombatSystem_h_

#include "../universe/Universe.h"

/** Contains information about the state of a combat before or after the combat
  * occurs. */
struct CombatInfo {
public:
    /** \name Structors */ //@{
    CombatInfo();                       ///< Default ctor
    CombatInfo(int system_id_);         ///< ctor taking \a system_id of system where combat occurs, and which will populate itself with all relevant fields and copies of objects based on the contents of that system
    //@}

    /** \name Accessors */ //@{
    const System*   GetSystem() const;  ///< returns System object in this CombatInfo's objects if one exists with id system_id
    //@}

    /** \name Mutators */ //@{
    void            Clear();            ///< cleans up contents
    System*         GetSystem();        ///< returns System object in this CombatInfo's objects if one exists with id system_id
    //@}

    int                             system_id;                      ///< ID of system where combat is occurring (could be INVALID_OBJECT_ID ?)
    std::set<int>                   empire_ids;                     ///< IDs of empires involved in combat
    ObjectMap                       objects;                        ///< actual state of objects relevant to combat
    std::map<int, ObjectMap>        empire_known_objects;           ///< each empire's latest known state of objects relevant to combat
    std::set<int>                   damaged_object_ids;             ///< ids of objects damaged during this battle
    std::set<int>                   destroyed_object_ids;           ///< ids of objects destroyed during this battle
    std::map<int, std::set<int> >   destroyed_object_knowers;       ///< indexed by empire ID, the set of ids of objects the empire knows were destroyed during the combat
    std::map<int, std::map<int, float> >attacker_target_damage;     ///< indexed by attacker id and target id, the damage dealt during combat
    Universe::EmpireObjectVisibilityMap empire_object_visibility;   ///< indexed by empire id and object id, the visibility level the empire has of each object.  may be increased during battle

private:
    void            GetEmpireIdsToSerialize(                std::set<int>&                          filtered_empire_ids,                int encoding_empire) const;
    void            GetObjectsToSerialize(                  ObjectMap&                              filtered_objects,                   int encoding_empire) const;
    void            GetEmpireKnownObjectsToSerialize(       std::map<int, ObjectMap>&               filtered_empire_known_objects,      int encoding_empire) const;
    void            GetDamagedObjectsToSerialize(           std::set<int>&                          filtered_damaged_objects,           int encoding_empire) const;
    void            GetDestroyedObjectsToSerialize(         std::set<int>&                          filtered_destroyed_objects,         int encoding_empire) const;
    void            GetDestroyedObjectKnowersToSerialize(   std::map<int, std::set<int> >&          filtered_destroyed_object_knowers,  int encoding_empire) const;
    void            GetAttackerTargetDamageToSerialize(     std::map<int, std::map<int, float> >&   filtered_attacker_target_damage,    int encoding_empire) const;
    void            GetEmpireObjectVisibilityToSerialize(   Universe::EmpireObjectVisibilityMap&    filtered_empire_object_visibility,  int encoding_empire) const;

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
    std::map<int, ObjectMap>            filtered_empire_known_objects;
    std::set<int>                       filtered_damaged_object_ids;
    std::set<int>                       filtered_destroyed_object_ids;
    std::map<int, std::set<int> >       filtered_destroyed_object_knowers;
    std::map<int, std::map<int, float> >filtered_attacker_target_damage;
    Universe::EmpireObjectVisibilityMap filtered_empire_object_visibility;

    GetEmpireIdsToSerialize(                filtered_empire_ids,                GetUniverse().EncodingEmpire());
    GetObjectsToSerialize(                  filtered_objects,                   GetUniverse().EncodingEmpire());
    GetEmpireKnownObjectsToSerialize(       filtered_empire_known_objects,      GetUniverse().EncodingEmpire());
    GetDamagedObjectsToSerialize(           filtered_damaged_object_ids,        GetUniverse().EncodingEmpire());
    GetDestroyedObjectsToSerialize(         filtered_destroyed_object_ids,      GetUniverse().EncodingEmpire());
    GetDestroyedObjectKnowersToSerialize(   filtered_destroyed_object_knowers,  GetUniverse().EncodingEmpire());
    GetAttackerTargetDamageToSerialize(     filtered_attacker_target_damage,    GetUniverse().EncodingEmpire());
    GetEmpireObjectVisibilityToSerialize(   filtered_empire_object_visibility,  GetUniverse().EncodingEmpire());

    ar  & BOOST_SERIALIZATION_NVP(system_id)
        & BOOST_SERIALIZATION_NVP(filtered_empire_ids)
        & BOOST_SERIALIZATION_NVP(filtered_objects)
        & BOOST_SERIALIZATION_NVP(filtered_empire_known_objects)
        & BOOST_SERIALIZATION_NVP(filtered_damaged_object_ids)
        & BOOST_SERIALIZATION_NVP(filtered_destroyed_object_ids)
        & BOOST_SERIALIZATION_NVP(filtered_destroyed_object_knowers)
        & BOOST_SERIALIZATION_NVP(filtered_attacker_target_damage)
        & BOOST_SERIALIZATION_NVP(filtered_empire_object_visibility);
}

template <class Archive>
void CombatInfo::load(Archive & ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(system_id)
        & BOOST_SERIALIZATION_NVP(empire_ids)
        & BOOST_SERIALIZATION_NVP(objects)
        & BOOST_SERIALIZATION_NVP(empire_known_objects)
        & BOOST_SERIALIZATION_NVP(damaged_object_ids)
        & BOOST_SERIALIZATION_NVP(destroyed_object_ids)
        & BOOST_SERIALIZATION_NVP(destroyed_object_knowers)
        & BOOST_SERIALIZATION_NVP(attacker_target_damage)
        & BOOST_SERIALIZATION_NVP(empire_object_visibility);
}

#endif // _CombatSystem_h_
