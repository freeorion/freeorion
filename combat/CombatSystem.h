#ifndef _CombatSystem_h_
#define _CombatSystem_h_

#include "../universe/Universe.h"
#include "../universe/ScriptingContext.h"
#include "CombatEvent.h"


/** Contains information about the state of a combat before or after the combat
  * occurs. */
struct CombatInfo {
public:
    CombatInfo() = default;
    CombatInfo(int system_id_, int turn_,
               const Universe::EmpireObjectVisibilityMap& vis_,
               ObjectMap& objects_,
               const EmpireManager::container_type& empires_,
               const Universe::EmpireObjectVisibilityTurnMap& empire_object_vis_turns_,
               const EmpireManager::DiploStatusMap& diplo_statuses_);

    /** Returns System object in this CombatInfo's objects if one exists with
        id system_id. */
    std::shared_ptr<const System> GetSystem() const;

    /** Returns System object in this CombatInfo's objects if one exists with
        id system_id. */
    std::shared_ptr<System> GetSystem();

    const EmpireManager::container_type&           empires{Empires().GetEmpires()}; // map from ID to empires, may include empires not actually participating in this combat
    const Universe::EmpireObjectVisibilityTurnMap& empire_object_vis_turns{GetUniverse().GetEmpireObjectVisibilityTurnMap()};
    const EmpireManager::DiploStatusMap&           diplo_statuses{Empires().GetDiplomaticStatuses()};

    std::unique_ptr<ObjectMap>          objects;                       ///< actual state of objects relevant to combat, filtered and copied for system where combat occurs
    Universe::EmpireObjectVisibilityMap empire_object_visibility;      ///< indexed by empire id and object id, the visibility level the empire has of each object.  may be increased during battle
    int                                 bout = 0;                      ///< current combat bout, used with CombatBout ValueRef for implementing bout dependent targeting. First combat bout is 1
    int                                 turn = INVALID_GAME_TURN;      ///< main game turn
    int                                 system_id = INVALID_OBJECT_ID; ///< ID of system where combat is occurring (could be INVALID_OBJECT_ID ?)
    std::set<int>                       empire_ids;                    ///< IDs of empires involved in combat
    std::set<int>                       damaged_object_ids;            ///< ids of objects damaged during this battle
    std::set<int>                       destroyed_object_ids;          ///< ids of objects destroyed during this battle
    std::map<int, std::set<int>>        destroyed_object_knowers;      ///< indexed by empire ID, the set of ids of objects the empire knows were destroyed during the combat
    std::vector<CombatEventPtr>         combat_events;                 ///< list of combat attack events that occur in combat

    float GetMonsterDetection() const;

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
