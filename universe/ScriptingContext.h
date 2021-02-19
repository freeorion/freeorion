#ifndef _ScriptingContext_h_
#define _ScriptingContext_h_


#include <boost/variant.hpp>
#include "Universe.h"
#include "../Empire/EmpireManager.h"
#include "../util/AppInterface.h"

struct CombatInfo;

struct ScriptingContext {
    typedef boost::variant<
        int, double, PlanetType, PlanetSize, ::PlanetEnvironment, StarType,
        UniverseObjectType, Visibility, std::string, std::vector<std::string>
    > CurrentValueVariant;

    ScriptingContext() :
        ScriptingContext(GetUniverse(), ::Empires(), GetGalaxySetupData(),
                         GetSpeciesManager(), GetSupplyManager())
    {}

    ScriptingContext(const ScriptingContext& parent_context,
                     std::shared_ptr<const UniverseObject> condition_local_candidate_) :
        source(                   parent_context.source),
        effect_target(            parent_context.effect_target),
        condition_root_candidate( parent_context.condition_root_candidate ?
                                      parent_context.condition_root_candidate :
                                      condition_local_candidate_), // if parent context doesn't already have a root candidate, the new local candidate is the root
        condition_local_candidate(std::move(condition_local_candidate_)),
        current_value(            parent_context.current_value),
        combat_bout(              parent_context.combat_bout),
        current_turn(             parent_context.current_turn),
        galaxy_setup_data(        parent_context.galaxy_setup_data),
        species(                  parent_context.species),
        supply(                   parent_context.supply),
        universe(                 parent_context.universe),
        const_universe(           parent_context.const_universe),
        objects(                  parent_context.objects),
        const_objects(            parent_context.const_objects),
        empire_object_vis(        parent_context.empire_object_vis),
        empire_object_vis_turns(  parent_context.empire_object_vis_turns),
        empires(                  parent_context.empires),
        const_empires(            parent_context.const_empires),
        diplo_statuses(           parent_context.diplo_statuses)
    {}

    ScriptingContext(std::shared_ptr<const UniverseObject> source_) :
        source(           std::move(source_)),
        galaxy_setup_data(GetGalaxySetupData()),
        species(          GetSpeciesManager()),
        supply(           GetSupplyManager()),
        universe(         &GetUniverse()),
        const_universe(   GetUniverse()),
        empires(          &(::Empires().GetEmpires())),
        const_empires(    const_cast<const EmpireManager&>(::Empires()).GetEmpires()),
        diplo_statuses(   ::Empires().GetDiplomaticStatuses())
    {}

    ScriptingContext(std::shared_ptr<const UniverseObject> source_,
                     const ScriptingContext& parent_context) :
        source(                   std::move(source_)),
        effect_target(            parent_context.effect_target),
        condition_root_candidate( parent_context.condition_root_candidate),
        condition_local_candidate(parent_context.condition_local_candidate),
        current_value(            parent_context.current_value),
        combat_bout(              parent_context.combat_bout),
        current_turn(             parent_context.current_turn),
        galaxy_setup_data(        parent_context.galaxy_setup_data),
        species(                  parent_context.species),
        supply(                   parent_context.supply),
        universe(                 parent_context.universe),
        const_universe(           parent_context.const_universe),
        objects(                  parent_context.objects),
        const_objects(            parent_context.const_objects),
        empire_object_vis(        parent_context.empire_object_vis),
        empire_object_vis_turns(  parent_context.empire_object_vis_turns),
        empires(                  parent_context.empires),
        const_empires(            parent_context.const_empires),
        diplo_statuses(           parent_context.diplo_statuses)
    {}

    ScriptingContext(const ScriptingContext& parent_context,
                     const CurrentValueVariant& current_value_) :
        source(                   parent_context.source),
        effect_target(            parent_context.effect_target),
        condition_root_candidate( parent_context.condition_root_candidate),
        condition_local_candidate(parent_context.condition_local_candidate),
        current_value(            current_value_),
        combat_bout(              parent_context.combat_bout),
        current_turn(             parent_context.current_turn),
        galaxy_setup_data(        parent_context.galaxy_setup_data),
        species(                  parent_context.species),
        supply(                   parent_context.supply),
        universe(                 parent_context.universe),
        const_universe(           parent_context.const_universe),
        objects(                  parent_context.objects),
        const_objects(            parent_context.const_objects),
        empire_object_vis(        parent_context.empire_object_vis),
        empire_object_vis_turns(  parent_context.empire_object_vis_turns),
        empires(                  parent_context.empires),
        const_empires(            parent_context.const_empires),
        diplo_statuses(           parent_context.diplo_statuses)
    {}

    ScriptingContext(const ScriptingContext& parent_context,
                     std::shared_ptr<UniverseObject> target_,
                     const CurrentValueVariant& current_value_) :
        source(                   parent_context.source),
        effect_target(            std::move(target_)),
        condition_root_candidate( parent_context.condition_root_candidate),
        condition_local_candidate(parent_context.condition_local_candidate),
        current_value(            current_value_),
        combat_bout(              parent_context.combat_bout),
        current_turn(             parent_context.current_turn),
        galaxy_setup_data(        parent_context.galaxy_setup_data),
        species(                  parent_context.species),
        supply(                   parent_context.supply),
        universe(                 parent_context.universe),
        const_universe(           parent_context.const_universe),
        objects(                  parent_context.objects),
        const_objects(            parent_context.const_objects),
        empire_object_vis(        parent_context.empire_object_vis),
        empire_object_vis_turns(  parent_context.empire_object_vis_turns),
        empires(                  parent_context.empires),
        const_empires(            parent_context.const_empires),
        diplo_statuses(           parent_context.diplo_statuses)
    {}

    ScriptingContext(std::shared_ptr<const UniverseObject> source_,
                     std::shared_ptr<UniverseObject> target_) :
        source(std::move(source_)),
        effect_target(std::move(target_)),
        galaxy_setup_data(GetGalaxySetupData()),
        species(          GetSpeciesManager()),
        supply(           GetSupplyManager()),
        universe(         &GetUniverse()),
        const_universe(   GetUniverse()),
        empires(          &(::Empires().GetEmpires())),
        const_empires(    const_cast<const EmpireManager&>(::Empires()).GetEmpires()),
        diplo_statuses(   ::Empires().GetDiplomaticStatuses())
    {}

    ScriptingContext(Universe& universe, EmpireManager& empires_,
                     const GalaxySetupData& galaxy_setup_data_ = GetGalaxySetupData(),
                     SpeciesManager& species_ = GetSpeciesManager(),
                     const SupplyManager& supply_ = GetSupplyManager()) :
        galaxy_setup_data(galaxy_setup_data_),
        species(          species_),
        supply(           supply_),
        universe(         &universe),
        const_universe(   universe),
        empires(          &(empires_.GetEmpires())),
        const_empires(    const_cast<const EmpireManager&>(empires_).GetEmpires()),
        diplo_statuses(   empires_.GetDiplomaticStatuses())
    {}

    explicit ScriptingContext(CombatInfo& info, // in CombatSystem.cpp
                              std::shared_ptr<UniverseObject> attacker = nullptr);

    ScriptingContext(const Universe& universe, const EmpireManager& empires_,
                     std::shared_ptr<const UniverseObject> source_ = nullptr,
                     std::shared_ptr<UniverseObject> target_ = nullptr,
                     const CurrentValueVariant& current_value_ = CurrentValueVariant()) :
        source(        std::move(source_)),
        effect_target( std::move(target_)),
        current_value( current_value_),
        universe(      nullptr),
        const_universe(universe),
        empires(       nullptr),
        const_empires( empires_.GetEmpires()),
        diplo_statuses(empires_.GetDiplomaticStatuses())
    {}



    // helper functions for accessing state in this context

    // immutable container of immutable objects
    const Universe& ContextUniverse() const { return const_universe; }

    // mutable container of mutable objects, not thread safe to modify
    Universe& ContextUniverse() {
        if (universe)
            return *universe;
        ErrorLogger() << "ScriptingContext::ContextUniverse() asked for undefined mutable Universe";
        throw std::runtime_error("ScriptingContext::ContextUniverse() asked for undefined mutable Universe");
    }

    // immutable container of immutable objects
    const ObjectMap& ContextObjects() const { return const_objects; }

    // mutable container of mutable objects, not thread safe to modify
    ObjectMap& ContextObjects() {
        if (objects)
            return *objects;
        ErrorLogger() << "ScriptingContext::ContextUniverse() asked for undefined mutable ObjectMap";
        throw std::runtime_error("ScriptingContext::ContextUniverse() asked for undefined mutable objects");
    }

    DiplomaticStatus ContextDiploStatus(int empire1, int empire2) const {
        if (empire1 == ALL_EMPIRES || empire2 == ALL_EMPIRES || empire1 == empire2)
            return DiplomaticStatus::INVALID_DIPLOMATIC_STATUS;
        auto it = diplo_statuses.find(std::make_pair(std::max(empire1, empire2), std::min(empire1, empire2)));
        return it == diplo_statuses.end() ? DiplomaticStatus::INVALID_DIPLOMATIC_STATUS : it->second;
    }

    // mutable empire not thread safe to modify
    std::shared_ptr<Empire> GetEmpire(int id) {
        if (!empires) {
            ErrorLogger() << "ScriptingContext::GetEmpire() asked for unavailable mutable Empire";
            return nullptr;
        }
        auto it = empires->find(id);
        return it == empires->end() ? nullptr : it->second;
    }

    std::shared_ptr<const Empire> GetEmpire(int id) const {
        auto it = const_empires.find(id);
        return it == const_empires.end() ? nullptr : it->second;
    }

    const EmpireManager::const_container_type& Empires() const
    { return const_empires; } // const container of const empires

    const EmpireManager::container_type& Empires() { // const container of mutable empires
        if (empires)
            return *empires;
        ErrorLogger() << "ScriptingContext::ContextUniverse() asked for undefined mutable empires";
        throw std::runtime_error("ScriptingContext::ContextUniverse() asked for undefined mutable empires");
    }

    // script evaluation local state, some of which may vary during evaluation of an expression
    std::shared_ptr<const UniverseObject> source;
    std::shared_ptr<UniverseObject>       effect_target;
    std::shared_ptr<const UniverseObject> condition_root_candidate;
    std::shared_ptr<const UniverseObject> condition_local_candidate;
    const CurrentValueVariant             current_value;

    // general gamestate info
    int                                            combat_bout = 0;
    int                                            current_turn = CurrentTurn();
    const GalaxySetupData&                         galaxy_setup_data{GetGalaxySetupData()};
    SpeciesManager&                                species{GetSpeciesManager()};
    const SupplyManager&                           supply{GetSupplyManager()};
private: // Universe and ObjectMap getters select one of these based on constness
    Universe*                                      universe = nullptr;
    const Universe&                                const_universe{universe ? *universe : GetUniverse()};
    ObjectMap*                                     objects = universe ? &(universe->Objects()) : nullptr;
    const ObjectMap&                               const_objects{objects ? *objects : const_universe.Objects()};
public:
    const Universe::EmpireObjectVisibilityMap&     empire_object_vis{const_universe.GetEmpireObjectVisibility()};
    const Universe::EmpireObjectVisibilityTurnMap& empire_object_vis_turns{const_universe.GetEmpireObjectVisibilityTurnMap()};
private:
    const EmpireManager::container_type*           empires = nullptr;
    const EmpireManager::const_container_type&     const_empires{const_cast<const EmpireManager&>(::Empires()).GetEmpires()};
public:
    const EmpireManager::DiploStatusMap&           diplo_statuses{::Empires().GetDiplomaticStatuses()};
};


#endif
