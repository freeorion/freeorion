// -*- C++ -*-
#ifndef _CombatSystem_h_
#define _CombatSystem_h_

#include "../universe/Universe.h"

/** Contains information about the state of a combat before or after the combat
  * occurs. */
struct CombatInfo
{
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

    int                             system_id;                  ///< ID of system where combat is occurring (could be INVALID_OBJECT_ID ?)
    std::set<int>                   empire_ids;                 ///< IDs of empires involved in combat
    ObjectMap                       objects;                    ///< actual state of objects relevant to combat
    std::map<int, ObjectMap>        empire_known_objects;       ///< each empire's latest known state of objects relevant to combat
    std::set<int>                   destroyed_object_ids;       ///< ids of objects destroyed during this battle
    std::map<int, std::set<int> >   destroyed_object_knowers;   ///< indexed by empire ID, the set of ids of objects the empire knows were destroyed during the combat

private:
    void            GetEmpireIdsToSerialize(                std::set<int>&                  filtered_empire_ids,                int encoding_empire) const;
    void            GetObjectsToSerialize(                  ObjectMap&                      filtered_objects,                   int encoding_empire) const;
    void            GetEmpireKnownObjectsToSerialize(       std::map<int, ObjectMap>&       filtered_empire_known_objects,      int encoding_empire) const;
    void            GetDestroyedObjectsToSerialize(         std::set<int>&                  filtered_destroyed_objects,         int encoding_empire) const;
    void            GetDestroyedObjectKnowersToSerialize(   std::map<int, std::set<int> >&  filtered_destroyed_object_knowers,  int encoding_empire) const;

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
    std::set<int>                   filtered_empire_ids;
    ObjectMap                       filtered_objects;
    std::map<int, ObjectMap>        filtered_empire_known_objects;
    std::set<int>                   filtered_destroyed_object_ids;
    std::map<int, std::set<int> >   filtered_destroyed_object_knowers;

    GetEmpireIdsToSerialize(                filtered_empire_ids,                Universe::s_encoding_empire);
    GetObjectsToSerialize(                  filtered_objects,                   Universe::s_encoding_empire);
    GetEmpireKnownObjectsToSerialize(       filtered_empire_known_objects,      Universe::s_encoding_empire);
    GetDestroyedObjectsToSerialize(         filtered_destroyed_object_ids,      Universe::s_encoding_empire);
    GetDestroyedObjectKnowersToSerialize(   filtered_destroyed_object_knowers,  Universe::s_encoding_empire);

    ar  & BOOST_SERIALIZATION_NVP(system_id)
        & BOOST_SERIALIZATION_NVP(filtered_empire_ids)
        & BOOST_SERIALIZATION_NVP(filtered_objects)
        & BOOST_SERIALIZATION_NVP(filtered_empire_known_objects)
        & BOOST_SERIALIZATION_NVP(filtered_destroyed_object_ids)
        & BOOST_SERIALIZATION_NVP(filtered_destroyed_object_knowers);
}

template <class Archive>
void CombatInfo::load(Archive & ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(system_id)
        & BOOST_SERIALIZATION_NVP(empire_ids)
        & BOOST_SERIALIZATION_NVP(objects)
        & BOOST_SERIALIZATION_NVP(empire_known_objects)
        & BOOST_SERIALIZATION_NVP(destroyed_object_ids)
        & BOOST_SERIALIZATION_NVP(destroyed_object_knowers);
}

#endif // _CombatSystem_h_
