// -*- C++ -*-
#ifndef _CombatSystem_h_
#define _CombatSystem_h_

#include "../universe/Universe.h"

/** Contains information about the state of a combat before or after the combat
  * occurs. */
struct CombatInfo
{
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

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Auto-resolves a battle. */
void AutoResolveCombat(CombatInfo& combat_info);

template <class Archive>
void CombatInfo::serialize(Archive& ar, const unsigned int version)
{
}

#endif // _CombatSystem_h_
