#ifndef _ScriptingContext_h_
#define _ScriptingContext_h_

#include "Universe.h"
#include "../util/AppInterface.h"

#include <boost/any.hpp>

#include <memory>

class UniverseObject;

/** combat/CombatInfo extends this ScriptingCombatInfo in order
  * to give Conditions and ValueRefs access to combat related data */
struct FO_COMMON_API ScriptingCombatInfo {
    ScriptingCombatInfo() {}
    ScriptingCombatInfo(int bout_, const Universe::EmpireObjectVisibilityMap& vis) :
        bout(bout_),
        empire_object_visibility(vis)
    {}

    int                                 bout = 0;                   ///< current combat bout, used with CombatBout ValueRef for implementing bout dependent targeting. First combat bout is 1
    ObjectMap                           objects;                    ///< actual state of objects relevant to combat
    Universe::EmpireObjectVisibilityMap empire_object_visibility;   ///< indexed by empire id and object id, the visibility level the empire has of each object.  may be increased during battle
};
const ScriptingCombatInfo EMPTY_COMBAT_INFO{};

struct ScriptingContext {
    explicit ScriptingContext() :
        objects(Objects()),
        const_objects(Objects())
    {}

    explicit ScriptingContext(const ObjectMap& const_objects_) :
        objects(Objects()),
        const_objects(const_objects_)
    {}

    explicit ScriptingContext(ObjectMap& objects_) :
        objects(objects_),
        const_objects(objects_)
    {}

    explicit ScriptingContext(std::shared_ptr<const UniverseObject> source_,
                              const ObjectMap& const_objects_ = Objects()) :
        source(source_),
        objects(Objects()),
        const_objects(const_objects_)
    {}

    ScriptingContext(std::shared_ptr<const UniverseObject> source_,
                     const ScriptingContext& parent_context) :
        source(                     source_),
        effect_target(              parent_context.effect_target),
        condition_root_candidate(   parent_context.condition_root_candidate),
        condition_local_candidate(  parent_context.condition_local_candidate),
        current_value(              parent_context.current_value),
        combat_info(                parent_context.combat_info),
        objects(                    parent_context.objects),
        const_objects(              parent_context.const_objects)
    {}

    ScriptingContext(std::shared_ptr<const UniverseObject> source_,
                     ScriptingCombatInfo& combat_info_) :
        source(source_),
        combat_info(combat_info_),
        objects(combat_info_.objects),
        const_objects(combat_info_.objects)
    {}

    ScriptingContext(std::shared_ptr<const UniverseObject> source_,
                     std::shared_ptr<UniverseObject> target_,
                     ObjectMap& objects_ = Objects()) :
        source(source_),
        effect_target(target_),
        objects(objects_),
        const_objects(objects_)
    {}

    ScriptingContext(const ScriptingContext& parent_context,
                     std::shared_ptr<UniverseObject> target_,
                     const boost::any& current_value_) :    // TODO: Rework this so only specific types are accepted
        source(                     parent_context.source),
        effect_target(              target_),
        condition_root_candidate(   parent_context.condition_root_candidate),
        condition_local_candidate(  parent_context.condition_local_candidate),
        current_value(              current_value_),
        combat_info(                parent_context.combat_info),
        objects(                    parent_context.objects),
        const_objects(              parent_context.const_objects)
    {}

    ScriptingContext(const ScriptingContext& parent_context,
                     const boost::any& current_value_) :    // TODO: Rework this so only specific types are accepted
        source(                     parent_context.source),
        effect_target(              parent_context.effect_target),
        condition_root_candidate(   parent_context.condition_root_candidate),
        condition_local_candidate(  parent_context.condition_local_candidate),
        current_value(              current_value_),
        combat_info(                parent_context.combat_info),
        objects(                    parent_context.objects),
        const_objects(              parent_context.const_objects)
    {}

    ScriptingContext(const ScriptingContext& parent_context,
                     std::shared_ptr<const UniverseObject> condition_local_candidate_) :
        source(                         parent_context.source),
        effect_target(                  parent_context.effect_target),
        condition_root_candidate(       parent_context.condition_root_candidate ?
                                            parent_context.condition_root_candidate :
                                            condition_local_candidate_),    // if parent context doesn't already have a root candidate, the new local candidate is the root
        condition_local_candidate(      condition_local_candidate_),        // new local candidate
        current_value(                  parent_context.current_value),
        combat_info(                    parent_context.combat_info),
        objects(                        parent_context.objects),
        const_objects(                  parent_context.const_objects)
    {}

    ScriptingContext(std::shared_ptr<const UniverseObject> source_,
                     std::shared_ptr<UniverseObject> target_,
                     const boost::any& current_value_,  // TODO: Rework this so only specific types are accepted
                     std::shared_ptr<const UniverseObject> condition_root_candidate_ = nullptr,
                     std::shared_ptr<const UniverseObject> condition_local_candidate_ = nullptr,
                     ObjectMap& objects_ = Objects()) :
        source(source_),
        condition_root_candidate(condition_root_candidate_),
        condition_local_candidate(condition_local_candidate_),
        current_value(current_value_),
        objects(objects_),
        const_objects(objects_)
    {}

    const ObjectMap&    ContextObjects() const  { return const_objects; }
    ObjectMap&          ContextObjects()        { return objects; }

    std::shared_ptr<const UniverseObject>   source;
    std::shared_ptr<UniverseObject>         effect_target;
    std::shared_ptr<const UniverseObject>   condition_root_candidate;
    std::shared_ptr<const UniverseObject>   condition_local_candidate;
    const boost::any                        current_value;
    const ScriptingCombatInfo&              combat_info = EMPTY_COMBAT_INFO;

private:
    ObjectMap&                              objects;
    const ObjectMap&                        const_objects;
};

#endif // _ScriptingContext_h_
