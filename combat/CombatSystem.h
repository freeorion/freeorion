#ifndef _CombatSystem_h_
#define _CombatSystem_h_

#include "../universe/Universe.h"
#include "../universe/ScriptingContext.h"
#include "../util/AppInterface.h"
#include "CombatEvent.h"


/** Contains information about the state of a combat before or after the combat
  * occurs. */
struct CombatInfo : public ScriptingCombatInfo {
public:
    CombatInfo() = default;
    CombatInfo(int system_id_, int turn_);  ///< ctor taking system id where combat occurs and game turn on which combat occurs

    /** Returns System object in this CombatInfo's objects if one exists with
        id system_id. */
    std::shared_ptr<const System> GetSystem() const;

    /** Returns System object in this CombatInfo's objects if one exists with
        id system_id. */
    std::shared_ptr<System> GetSystem();

    int                                 turn = INVALID_GAME_TURN;       ///< main game turn
    int                                 system_id = INVALID_OBJECT_ID;  ///< ID of system where combat is occurring (could be INVALID_OBJECT_ID ?)
    std::set<int>                       empire_ids;                     ///< IDs of empires involved in combat
    std::set<int>                       damaged_object_ids;             ///< ids of objects damaged during this battle
    std::set<int>                       destroyed_object_ids;           ///< ids of objects destroyed during this battle
    std::map<int, std::set<int>>        destroyed_object_knowers;       ///< indexed by empire ID, the set of ids of objects the empire knows were destroyed during the combat
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

    template <typename Archive>
    friend void serialize(Archive&, CombatInfo&, unsigned int const);
};

/** Auto-resolves a battle. */
void AutoResolveCombat(CombatInfo& combat_info);



#endif
