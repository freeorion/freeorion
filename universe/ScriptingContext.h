#ifndef _ScriptingContext_h_
#define _ScriptingContext_h_


#include <boost/variant.hpp>
#include "Universe.h"
#include "../Empire/EmpireManager.h"
#include "../util/AppInterface.h"


class UniverseObject;


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
                              const EmpireManager::DiploStatusMap& diplo_statuses_ = Empires().GetDiplomaticStatuses()) :
        empire_object_vis(empire_object_vis_),
        const_objects(const_objects_),
        empire_object_vis_turns(empire_object_vis_turns_),
        empires(empires_),
        diplo_statuses(diplo_statuses_)
    {}

    explicit ScriptingContext(ObjectMap& objects_,
                              const Universe::EmpireObjectVisibilityMap&
                                  empire_object_vis_ = GetUniverse().GetEmpireObjectVisibility(),
                              const Universe::EmpireObjectVisibilityTurnMap&
                                  empire_object_vis_turns_ = GetUniverse().GetEmpireObjectVisibilityTurnMap(),
                              const EmpireManager::container_type& empires_ = Empires().GetEmpires(),
                              const EmpireManager::DiploStatusMap& diplo_statuses_ = Empires().GetDiplomaticStatuses()) :
        empire_object_vis(empire_object_vis_),
        objects(objects_),
        const_objects(objects_),
        empire_object_vis_turns(empire_object_vis_turns_),
        empires(empires_),
        diplo_statuses(diplo_statuses_)
    {}

    explicit ScriptingContext(std::shared_ptr<const UniverseObject> source_,
                              const ObjectMap& const_objects_ = Objects(),
                              const Universe::EmpireObjectVisibilityMap&
                                  empire_object_vis_ = GetUniverse().GetEmpireObjectVisibility(),
                              const Universe::EmpireObjectVisibilityTurnMap&
                                  empire_object_vis_turns_ = GetUniverse().GetEmpireObjectVisibilityTurnMap(),
                              const EmpireManager::container_type& empires_ = Empires().GetEmpires(),
                              const EmpireManager::DiploStatusMap& diplo_statuses_ = Empires().GetDiplomaticStatuses()) :
        source(std::move(source_)),
        empire_object_vis(empire_object_vis_),
        const_objects(const_objects_),
        empire_object_vis_turns(empire_object_vis_turns_),
        empires(empires_),
        diplo_statuses(diplo_statuses_)
    {}

    ScriptingContext(std::shared_ptr<const UniverseObject> source_,
                     const ScriptingContext& parent_context) :
        source(                     std::move(source_)),
        effect_target(              parent_context.effect_target),
        condition_root_candidate(   parent_context.condition_root_candidate),
        condition_local_candidate(  parent_context.condition_local_candidate),
        current_value(              parent_context.current_value),
        empire_object_vis(          parent_context.empire_object_vis),
        objects(                    parent_context.objects),
        const_objects(              parent_context.const_objects),
        empire_object_vis_turns(    parent_context.empire_object_vis_turns),
        empires(                    parent_context.empires),
        diplo_statuses(             parent_context.diplo_statuses)
    {}

    ScriptingContext(std::shared_ptr<const UniverseObject> source_,
                     std::shared_ptr<UniverseObject> target_,
                     ObjectMap& objects_ = Objects(),
                     const Universe::EmpireObjectVisibilityMap&
                         empire_object_vis_ = GetUniverse().GetEmpireObjectVisibility(),
                     const Universe::EmpireObjectVisibilityTurnMap&
                         empire_object_vis_turns_ = GetUniverse().GetEmpireObjectVisibilityTurnMap(),
                     const EmpireManager::container_type& empires_ = Empires().GetEmpires(),
                     const EmpireManager::DiploStatusMap& diplo_statuses_ = Empires().GetDiplomaticStatuses()) :
        source(std::move(source_)),
        effect_target(std::move(target_)),
        empire_object_vis(empire_object_vis_),
        objects(objects_),
        const_objects(objects_),
        empire_object_vis_turns(empire_object_vis_turns_),
        empires(empires_),
        diplo_statuses(diplo_statuses_)
    {}

    ScriptingContext(std::shared_ptr<const UniverseObject> source_,
                     std::shared_ptr<UniverseObject> target_,
                     const ObjectMap& const_objects_,
                     const Universe::EmpireObjectVisibilityMap&
                         empire_object_vis_ = GetUniverse().GetEmpireObjectVisibility(),
                     const Universe::EmpireObjectVisibilityTurnMap&
                         empire_object_vis_turns_ = GetUniverse().GetEmpireObjectVisibilityTurnMap(),
                     const EmpireManager::container_type& empires_ = Empires().GetEmpires(),
                     const EmpireManager::DiploStatusMap& diplo_statuses_ = Empires().GetDiplomaticStatuses()) :
        source(std::move(source_)),
        effect_target(std::move(target_)),
        empire_object_vis(empire_object_vis_),
        const_objects(const_objects_),
        empire_object_vis_turns(empire_object_vis_turns_),
        empires(empires_),
        diplo_statuses(diplo_statuses_)
    {}

    ScriptingContext(const ScriptingContext& parent_context,
                     std::shared_ptr<UniverseObject> target_,
                     const CurrentValueVariant& current_value_) :
        source(                     parent_context.source),
        effect_target(              std::move(target_)),
        condition_root_candidate(   parent_context.condition_root_candidate),
        condition_local_candidate(  parent_context.condition_local_candidate),
        current_value(              current_value_),
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
                     const EmpireManager::DiploStatusMap& diplo_statuses_ = Empires().GetDiplomaticStatuses()) :
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
        diplo_statuses(diplo_statuses_)
    {}


    // helper functions for accessing state in this context
    const ObjectMap&    ContextObjects() const  { return const_objects; } // immutable container of immutable objects
    ObjectMap&          ContextObjects()        { return objects; }       // mutable container of mutable objects

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


    // script evaluation local state, some of which may vary during evaluation of an expression
    std::shared_ptr<const UniverseObject> source;
    std::shared_ptr<UniverseObject>       effect_target;
    std::shared_ptr<const UniverseObject> condition_root_candidate;
    std::shared_ptr<const UniverseObject> condition_local_candidate;
    const CurrentValueVariant             current_value;

    // general gamestate info
    int                                            combat_bout = 0;
    const Universe::EmpireObjectVisibilityMap&     empire_object_vis{GetUniverse().GetEmpireObjectVisibility()}; // immutable container and values
    ObjectMap&                                     objects{Objects()};
    const ObjectMap&                               const_objects{Objects()};
    const Universe::EmpireObjectVisibilityTurnMap& empire_object_vis_turns{GetUniverse().GetEmpireObjectVisibilityTurnMap()}; // immutable container and values
    const EmpireManager::container_type&           empires{Empires().GetEmpires()};                   // immutable container of mutable empires
    const EmpireManager::DiploStatusMap&           diplo_statuses{Empires().GetDiplomaticStatuses()}; // immutable value reference
};


#endif
