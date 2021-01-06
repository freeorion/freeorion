#ifndef _ScriptingContext_h_
#define _ScriptingContext_h_


#include <boost/variant.hpp>
#include "Universe.h"
#include "../Empire/EmpireManager.h"
#include "../util/AppInterface.h"


class UniverseObject;

/** combat/CombatInfo extends this ScriptingCombatInfo in order
  * to give Conditions and ValueRefs access to combat related data */
struct FO_COMMON_API ScriptingCombatInfo {
    ScriptingCombatInfo() = default;
    explicit ScriptingCombatInfo(const Universe::EmpireObjectVisibilityMap& vis) :
        empire_object_visibility(vis)
    {}

    int                                 bout = 0;                   ///< current combat bout, used with CombatBout ValueRef for implementing bout dependent targeting. First combat bout is 1
    ObjectMap                           objects;                    ///< actual state of objects relevant to combat
    Universe::EmpireObjectVisibilityMap empire_object_visibility;   ///< indexed by empire id and object id, the visibility level the empire has of each object.  may be increased during battle
};

struct ScriptingContext {
    typedef boost::variant<
        int, double, PlanetType, PlanetSize, ::PlanetEnvironment, StarType,
        UniverseObjectType, Visibility, std::string, std::vector<std::string>
    > CurrentValueVariant;

    ScriptingContext() = default;

    explicit ScriptingContext(const ObjectMap& const_objects_,
                              const Universe::EmpireObjectVisibilityMap&
                                  empire_object_vis_ = GetUniverse().GetEmpireObjectVisibility(),
                              const Universe::EmpireObjectVisibilityTurnMap&
                                  empire_object_vis_turns_ = GetUniverse().GetEmpireObjectVisibilityTurnMap(),
                              const EmpireManager::container_type& empires_ = Empires().GetEmpires(),
                              EmpireManager::DiploStatusMap diplo_statuses_ = Empires().GetDiplomaticStatuses()) :
        empire_object_vis(empire_object_vis_),
        const_objects(const_objects_),
        empire_object_vis_turns(empire_object_vis_turns_),
        empires(empires_),
        diplo_statuses(std::move(diplo_statuses_))
    {}

    explicit ScriptingContext(ObjectMap& objects_,
                              const Universe::EmpireObjectVisibilityMap&
                                  empire_object_vis_ = GetUniverse().GetEmpireObjectVisibility(),
                              const Universe::EmpireObjectVisibilityTurnMap&
                                  empire_object_vis_turns_ = GetUniverse().GetEmpireObjectVisibilityTurnMap(),
                              const EmpireManager::container_type& empires_ = Empires().GetEmpires(),
                              EmpireManager::DiploStatusMap diplo_statuses_ = Empires().GetDiplomaticStatuses()) :
        empire_object_vis(empire_object_vis_),
        objects(objects_),
        const_objects(objects_),
        empire_object_vis_turns(empire_object_vis_turns_),
        empires(empires_),
        diplo_statuses(std::move(diplo_statuses_))
    {}

    explicit ScriptingContext(std::shared_ptr<const UniverseObject> source_,
                              const ObjectMap& const_objects_ = Objects(),
                              const Universe::EmpireObjectVisibilityMap&
                                  empire_object_vis_ = GetUniverse().GetEmpireObjectVisibility(),
                              const Universe::EmpireObjectVisibilityTurnMap&
                                  empire_object_vis_turns_ = GetUniverse().GetEmpireObjectVisibilityTurnMap(),
                              const EmpireManager::container_type& empires_ = Empires().GetEmpires(),
                              EmpireManager::DiploStatusMap diplo_statuses_ = Empires().GetDiplomaticStatuses()) :
        source(std::move(source_)),
        empire_object_vis(empire_object_vis_),
        const_objects(const_objects_),
        empire_object_vis_turns(empire_object_vis_turns_),
        empires(empires_),
        diplo_statuses(std::move(diplo_statuses_))
    {}

    ScriptingContext(std::shared_ptr<const UniverseObject> source_,
                     const ScriptingContext& parent_context) :
        source(                     std::move(source_)),
        effect_target(              parent_context.effect_target),
        condition_root_candidate(   parent_context.condition_root_candidate),
        condition_local_candidate(  parent_context.condition_local_candidate),
        current_value(              parent_context.current_value),
        combat_info(                parent_context.combat_info),
        empire_object_vis(          parent_context.empire_object_vis),
        objects(                    parent_context.objects),
        const_objects(              parent_context.const_objects),
        empire_object_vis_turns(    parent_context.empire_object_vis_turns),
        empires(                    parent_context.empires),
        diplo_statuses(             parent_context.diplo_statuses)
    {}

    ScriptingContext(std::shared_ptr<const UniverseObject> source_,
                     ScriptingCombatInfo& combat_info_,
                     const Universe::EmpireObjectVisibilityTurnMap&
                         empire_object_vis_turns_ = GetUniverse().GetEmpireObjectVisibilityTurnMap(),
                     const EmpireManager::container_type& empires_ = Empires().GetEmpires(),
                     EmpireManager::DiploStatusMap diplo_statuses_ = Empires().GetDiplomaticStatuses()) :
        source(std::move(source_)),
        combat_info(combat_info_),
        empire_object_vis(combat_info_.empire_object_visibility),
        objects(combat_info_.objects),
        const_objects(combat_info_.objects),
        empire_object_vis_turns(empire_object_vis_turns_),
        empires(empires_),
        diplo_statuses(std::move(diplo_statuses_))
    {}

    ScriptingContext(std::shared_ptr<const UniverseObject> source_,
                     std::shared_ptr<UniverseObject> target_,
                     ObjectMap& objects_ = Objects(),
                     const Universe::EmpireObjectVisibilityMap&
                         empire_object_vis_ = GetUniverse().GetEmpireObjectVisibility(),
                     const Universe::EmpireObjectVisibilityTurnMap&
                         empire_object_vis_turns_ = GetUniverse().GetEmpireObjectVisibilityTurnMap(),
                     const EmpireManager::container_type& empires_ = Empires().GetEmpires(),
                     EmpireManager::DiploStatusMap diplo_statuses_ = Empires().GetDiplomaticStatuses()) :
        source(std::move(source_)),
        effect_target(std::move(target_)),
        empire_object_vis(empire_object_vis_),
        objects(objects_),
        const_objects(objects_),
        empire_object_vis_turns(empire_object_vis_turns_),
        empires(empires_),
        diplo_statuses(std::move(diplo_statuses_))
    {}

    ScriptingContext(std::shared_ptr<const UniverseObject> source_,
                     std::shared_ptr<UniverseObject> target_,
                     const ObjectMap& const_objects_,
                     const Universe::EmpireObjectVisibilityMap&
                         empire_object_vis_ = GetUniverse().GetEmpireObjectVisibility(),
                     const Universe::EmpireObjectVisibilityTurnMap&
                         empire_object_vis_turns_ = GetUniverse().GetEmpireObjectVisibilityTurnMap(),
                     const EmpireManager::container_type& empires_ = Empires().GetEmpires(),
                     EmpireManager::DiploStatusMap diplo_statuses_ = Empires().GetDiplomaticStatuses()) :
        source(std::move(source_)),
        effect_target(std::move(target_)),
        empire_object_vis(empire_object_vis_),
        const_objects(const_objects_),
        empire_object_vis_turns(empire_object_vis_turns_),
        empires(empires_),
        diplo_statuses(std::move(diplo_statuses_))
    {}

    ScriptingContext(const ScriptingContext& parent_context,
                     std::shared_ptr<UniverseObject> target_,
                     const CurrentValueVariant& current_value_) :
        source(                     parent_context.source),
        effect_target(              std::move(target_)),
        condition_root_candidate(   parent_context.condition_root_candidate),
        condition_local_candidate(  parent_context.condition_local_candidate),
        current_value(              current_value_),
        combat_info(                parent_context.combat_info),
        empire_object_vis(          parent_context.empire_object_vis),
        objects(                    parent_context.objects),
        const_objects(              parent_context.const_objects),
        empire_object_vis_turns(    parent_context.empire_object_vis_turns),
        empires(                    parent_context.empires),
        diplo_statuses(             parent_context.diplo_statuses)
    {}

    ScriptingContext(const ScriptingContext& parent_context,
                     const CurrentValueVariant& current_value_) :
        source(                     parent_context.source),
        effect_target(              parent_context.effect_target),
        condition_root_candidate(   parent_context.condition_root_candidate),
        condition_local_candidate(  parent_context.condition_local_candidate),
        current_value(              current_value_),
        combat_info(                parent_context.combat_info),
        empire_object_vis(          parent_context.empire_object_vis),
        objects(                    parent_context.objects),
        const_objects(              parent_context.const_objects),
        empire_object_vis_turns(    parent_context.empire_object_vis_turns),
        empires(                    parent_context.empires),
        diplo_statuses(             parent_context.diplo_statuses)
    {}

    ScriptingContext(const ScriptingContext& parent_context,
                     std::shared_ptr<const UniverseObject> condition_local_candidate_) :
        source(                     parent_context.source),
        effect_target(              parent_context.effect_target),
        condition_root_candidate(   parent_context.condition_root_candidate ?
                                        parent_context.condition_root_candidate :
                                        condition_local_candidate_),// if parent context doesn't already have a root candidate, the new local candidate is the root
        condition_local_candidate(  condition_local_candidate_),    // new local candidate
        current_value(              parent_context.current_value),
        combat_info(                parent_context.combat_info),
        empire_object_vis(          parent_context.empire_object_vis),
        objects(                    parent_context.objects),
        const_objects(              parent_context.const_objects),
        empire_object_vis_turns(    parent_context.empire_object_vis_turns),
        empires(                    parent_context.empires),
        diplo_statuses(             parent_context.diplo_statuses)
    {}

    ScriptingContext(std::shared_ptr<const UniverseObject> source_,
                     std::shared_ptr<UniverseObject> target_,
                     const CurrentValueVariant& current_value_,
                     std::shared_ptr<const UniverseObject> condition_root_candidate_ = nullptr,
                     std::shared_ptr<const UniverseObject> condition_local_candidate_ = nullptr,
                     ObjectMap& objects_ = Objects(),
                     const Universe::EmpireObjectVisibilityMap&
                         empire_object_vis_ = GetUniverse().GetEmpireObjectVisibility(),
                     const Universe::EmpireObjectVisibilityTurnMap&
                         empire_object_vis_turns_ = GetUniverse().GetEmpireObjectVisibilityTurnMap(),
                     const EmpireManager::container_type& empires_ = Empires().GetEmpires(),
                     EmpireManager::DiploStatusMap diplo_statuses_ = Empires().GetDiplomaticStatuses()) :
        source(std::move(source_)),
        effect_target(std::move(target_)),
        condition_root_candidate(std::move(condition_root_candidate_)),
        condition_local_candidate(std::move(condition_local_candidate_)),
        current_value(current_value_),
        empire_object_vis(empire_object_vis_),
        objects(objects_),
        const_objects(objects_),
        empire_object_vis_turns(empire_object_vis_turns_),
        empires(empires_),
        diplo_statuses(std::move(diplo_statuses_))
    {}

    const ObjectMap&    ContextObjects() const  { return const_objects; }   // immutable container of immutable objects
    ObjectMap&          ContextObjects()        { return objects; }         // mutable container of mutable objects

    DiplomaticStatus    ContextDiploStatus(int empire1, int empire2) const {
        if (empire1 == ALL_EMPIRES || empire2 == ALL_EMPIRES || empire1 == empire2)
            return DiplomaticStatus::INVALID_DIPLOMATIC_STATUS;
        auto it = diplo_statuses.find(std::make_pair(std::max(empire1, empire2), std::min(empire1, empire2)));
        return it == diplo_statuses.end() ? DiplomaticStatus::INVALID_DIPLOMATIC_STATUS : it->second;
    }

    std::shared_ptr<Empire> GetEmpire(int id) {
        auto it = empires.find(id);
        return it == empires.end() ? nullptr : it->second;
    }
    std::shared_ptr<const Empire> GetEmpire(int id) const {
        auto it = empires.find(id);
        return it == empires.end() ? nullptr : it->second;
    }

    std::shared_ptr<const UniverseObject>   source;
    std::shared_ptr<UniverseObject>         effect_target;
    std::shared_ptr<const UniverseObject>   condition_root_candidate;
    std::shared_ptr<const UniverseObject>   condition_local_candidate;
    const CurrentValueVariant               current_value;

    const ScriptingCombatInfo&                      combat_info{EmptyCombatInfo()};
    const Universe::EmpireObjectVisibilityMap&      empire_object_vis{GetUniverse().GetEmpireObjectVisibility()}; // immutable container and values

    ObjectMap&                                      objects{Objects()};
    const ObjectMap&                                const_objects{Objects()};
    const Universe::EmpireObjectVisibilityTurnMap&  empire_object_vis_turns{GetUniverse().GetEmpireObjectVisibilityTurnMap()}; // immutable container and values
    const EmpireManager::container_type&            empires{Empires().GetEmpires()};                   // immutable container of mutable empires
    const EmpireManager::DiploStatusMap             diplo_statuses{Empires().GetDiplomaticStatuses()}; // by-value copied at initialization

private:
    static const ScriptingCombatInfo& EmptyCombatInfo() {
        static const ScriptingCombatInfo EMPTY_COMBAT_INFO;
        return EMPTY_COMBAT_INFO;
    }
};


#endif
