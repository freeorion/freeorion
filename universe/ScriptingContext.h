#ifndef _ScriptingContext_h_
#define _ScriptingContext_h_

#include "Universe.h"

#include <boost/any.hpp>

#include <memory>

class UniverseObject;

struct ScriptingContext {
    /** Empty context.  Useful for evaluating ValueRef::Constant that don't
      * depend on their context. */
    ScriptingContext()
    {}

    /** Context with only a source object.  Useful for evaluating effectsgroup
      * scope and activation conditions that have no external candidates or
      * effect target to propagate. */
    explicit ScriptingContext(std::shared_ptr<const UniverseObject> source_) :
        source(source_)
    {}

    /** Context with source and visibility map to use when evalulating Visiblity
      * conditions. Useful in combat resolution when the visibility of objects
      * may be different from the overall universe visibility. */
    ScriptingContext(std::shared_ptr<const UniverseObject> source_,
                     const Universe::EmpireObjectVisibilityMap& visibility_map) :
        source(source_),
        empire_object_vis_map_override(visibility_map)
    {}

    ScriptingContext(std::shared_ptr<const UniverseObject> source_,
                     std::shared_ptr<UniverseObject> target_) :
        source(source_),
        effect_target(target_)
    {}

    ScriptingContext(std::shared_ptr<const UniverseObject> source_,
                     std::shared_ptr<UniverseObject> target_,
                     const boost::any& current_value_) :
        source(source_),
        effect_target(target_),
        current_value(current_value_)
    {}

    /** For evaluating ValueRef in an Effect::Execute function.  Keeps input
      * context, but specifies the current value. */
    ScriptingContext(const ScriptingContext& parent_context,
                     const boost::any& current_value_) :
        source(parent_context.source),
        effect_target(parent_context.effect_target),
        condition_root_candidate(parent_context.condition_root_candidate),
        condition_local_candidate(parent_context.condition_local_candidate),
        current_value(current_value_),
        empire_object_vis_map_override(parent_context.empire_object_vis_map_override)
    {}

    /** For recusrive evaluation of Conditions.  Keeps source and effect_target
      * from input context, but sets local candidate with input object, and if
      * there is no root candidate in the parent context, then the input object
      * becomes the root candidate. */
    ScriptingContext(const ScriptingContext& parent_context,
                     std::shared_ptr<const UniverseObject> condition_local_candidate_) :
        source(                         parent_context.source),
        effect_target(                  parent_context.effect_target),
        condition_root_candidate(       parent_context.condition_root_candidate ?
                                            parent_context.condition_root_candidate :
                                            condition_local_candidate_),    // if parent context doesn't already have a root candidate, the new local candidate is the root
        condition_local_candidate(      condition_local_candidate_),        // new local candidate
        current_value(                  parent_context.current_value),
        empire_object_vis_map_override( parent_context.empire_object_vis_map_override)
    {}

    ScriptingContext(std::shared_ptr<const UniverseObject> source_,
                     std::shared_ptr<UniverseObject> target_,
                     const boost::any& current_value_,
                     std::shared_ptr<const UniverseObject> condition_root_candidate_,
                     std::shared_ptr<const UniverseObject> condition_local_candidate_) :
        source(source_),
        condition_root_candidate(condition_root_candidate_),
        condition_local_candidate(condition_local_candidate_),
        current_value(current_value_)
    {}

    std::shared_ptr<const UniverseObject>   source;
    std::shared_ptr<UniverseObject>         effect_target;
    std::shared_ptr<const UniverseObject>   condition_root_candidate;
    std::shared_ptr<const UniverseObject>   condition_local_candidate;
    const boost::any                        current_value;
    Universe::EmpireObjectVisibilityMap     empire_object_vis_map_override;
};

#endif // _ScriptingContext_h_
