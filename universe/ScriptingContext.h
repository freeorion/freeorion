#ifndef _ScriptingContext_h_
#define _ScriptingContext_h_


#include <boost/variant.hpp>
#include "Universe.h"
#include "../Empire/EmpireManager.h"
#include "../util/AppInterface.h"

struct CombatInfo;

struct [[nodiscard]] ScriptingContext {
    typedef boost::variant<
        int, double, PlanetType, PlanetSize, ::PlanetEnvironment, StarType,
        UniverseObjectType, Visibility, std::string, std::vector<std::string>
    > CurrentValueVariant;
    inline static const CurrentValueVariant DEFAULT_CURRENT_VALUE{0};

    ScriptingContext() noexcept :
        ScriptingContext(GetUniverse(), ::Empires(), GetGalaxySetupData(),
                         GetSpeciesManager(), GetSupplyManager())
    {}

    ScriptingContext(const ScriptingContext& parent_context,
                     const UniverseObject* condition_local_candidate_) noexcept :
        source(                   parent_context.source),
        effect_target(            parent_context.effect_target),
        condition_root_candidate( parent_context.condition_root_candidate ?
                                      parent_context.condition_root_candidate :
                                      condition_local_candidate_), // if parent context doesn't already have a root candidate, the new local candidate is the root
        condition_local_candidate(condition_local_candidate_),
        current_value(            parent_context.current_value),
        combat_bout(              parent_context.combat_bout),
        current_turn(             parent_context.current_turn),
        in_design_id(             parent_context.in_design_id),
        production_block_size(    parent_context.production_block_size),
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

    explicit ScriptingContext(const UniverseObject* source_) noexcept :
        source(           source_),
        galaxy_setup_data(GetGalaxySetupData()),
        species(          GetSpeciesManager()),
        supply(           GetSupplyManager()),
        universe(         &GetUniverse()),
        const_universe(   GetUniverse()),
        empires(          &(::Empires())),
        const_empires(    ::Empires()),
        diplo_statuses(   ::Empires().GetDiplomaticStatuses())
    {}

    ScriptingContext(const UniverseObject* source_,
                     const ScriptingContext& parent_context) noexcept :
        source(                   source_),
        effect_target(            parent_context.effect_target),
        condition_root_candidate( parent_context.condition_root_candidate),
        condition_local_candidate(parent_context.condition_local_candidate),
        current_value(            parent_context.current_value),
        combat_bout(              parent_context.combat_bout),
        current_turn(             parent_context.current_turn),
        in_design_id(             parent_context.in_design_id),
        production_block_size(    parent_context.production_block_size),
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
                     const Universe::EmpireObjectVisibilityMap& vis,
                     const Universe::EmpireObjectVisibilityTurnMap& vis_turns,
                     const UniverseObject* source_ = nullptr,
                     UniverseObject* target_ = nullptr) noexcept :
        source(                   source_),
        effect_target(            target_),
        condition_root_candidate( parent_context.condition_root_candidate),
        condition_local_candidate(parent_context.condition_local_candidate),
        current_value(            parent_context.current_value),
        combat_bout(              parent_context.combat_bout),
        current_turn(             parent_context.current_turn),
        in_design_id(             parent_context.in_design_id),
        production_block_size(    parent_context.production_block_size),
        galaxy_setup_data(        parent_context.galaxy_setup_data),
        species(                  parent_context.species),
        supply(                   parent_context.supply),
        universe(                 parent_context.universe),
        const_universe(           parent_context.const_universe),
        objects(                  parent_context.objects),
        const_objects(            parent_context.const_objects),
        empire_object_vis(        vis),
        empire_object_vis_turns(  vis_turns),
        empires(                  parent_context.empires),
        const_empires(            parent_context.const_empires),
        diplo_statuses(           parent_context.diplo_statuses)
    {}

    ScriptingContext(const ScriptingContext& parent_context,
                     const CurrentValueVariant& current_value_) noexcept :
        source(                   parent_context.source),
        effect_target(            parent_context.effect_target),
        condition_root_candidate( parent_context.condition_root_candidate),
        condition_local_candidate(parent_context.condition_local_candidate),
        current_value(            current_value_),
        combat_bout(              parent_context.combat_bout),
        current_turn(             parent_context.current_turn),
        in_design_id(             parent_context.in_design_id),
        production_block_size(    parent_context.production_block_size),
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

    ScriptingContext(ScriptingContext&& parent_context,
                     const CurrentValueVariant& current_value_) noexcept :
        source(                   parent_context.source),
        effect_target(            parent_context.effect_target),
        condition_root_candidate( parent_context.condition_root_candidate),
        condition_local_candidate(parent_context.condition_local_candidate),
        current_value(            current_value_),
        combat_bout(              parent_context.combat_bout),
        current_turn(             parent_context.current_turn),
        in_design_id(             parent_context.in_design_id),
        production_block_size(    parent_context.production_block_size),
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

    // disable implicit conversions to CurrentValueVariant
    ScriptingContext(ScriptingContext&&, int) = delete;
    ScriptingContext(const ScriptingContext&, int) = delete;
    ScriptingContext(ScriptingContext&&, double) = delete;
    ScriptingContext(const ScriptingContext&, double) = delete;
    ScriptingContext(ScriptingContext&&, PlanetType) = delete;
    ScriptingContext(const ScriptingContext&, PlanetType) = delete;
    ScriptingContext(ScriptingContext&&, PlanetSize) = delete;
    ScriptingContext(const ScriptingContext&, PlanetSize) = delete;
    ScriptingContext(ScriptingContext&&, ::PlanetEnvironment) = delete;
    ScriptingContext(const ScriptingContext&, ::PlanetEnvironment) = delete;
    ScriptingContext(ScriptingContext&&, StarType) = delete;
    ScriptingContext(const ScriptingContext&, StarType) = delete;
    ScriptingContext(ScriptingContext&&, UniverseObjectType) = delete;
    ScriptingContext(const ScriptingContext&, UniverseObjectType) = delete;
    ScriptingContext(ScriptingContext&&, Visibility) = delete;
    ScriptingContext(const ScriptingContext&, Visibility) = delete;
    ScriptingContext(ScriptingContext&&, std::string) = delete;
    ScriptingContext(const ScriptingContext&, std::string) = delete;
    ScriptingContext(ScriptingContext&&, std::vector<std::string>) = delete;
    ScriptingContext(const ScriptingContext&, std::vector<std::string>) = delete;


    ScriptingContext(const ScriptingContext& parent_context,
                     const UniverseObject* source_,
                     UniverseObject* target_,
                     int in_design_id_, int production_block_size_) noexcept :
        source(                   source_),
        effect_target(            target_),
        condition_root_candidate( parent_context.condition_root_candidate),
        condition_local_candidate(parent_context.condition_local_candidate),
        current_value(            parent_context.current_value),
        combat_bout(              parent_context.combat_bout),
        current_turn(             parent_context.current_turn),
        in_design_id(             in_design_id_),
        production_block_size(    production_block_size_),
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
                     UniverseObject* target_,
                     const CurrentValueVariant& current_value_) noexcept :
        source(                   parent_context.source),
        effect_target(            target_),
        condition_root_candidate( parent_context.condition_root_candidate),
        condition_local_candidate(parent_context.condition_local_candidate),
        current_value(            current_value_),
        combat_bout(              parent_context.combat_bout),
        current_turn(             parent_context.current_turn),
        in_design_id(             parent_context.in_design_id),
        production_block_size(    parent_context.production_block_size),
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

    ScriptingContext(ScriptingContext&& parent_context,
                     UniverseObject* target_,
                     const CurrentValueVariant& current_value_) noexcept :
        source(                   parent_context.source),
        effect_target(            target_),
        condition_root_candidate( parent_context.condition_root_candidate),
        condition_local_candidate(parent_context.condition_local_candidate),
        current_value(            current_value_),
        combat_bout(              parent_context.combat_bout),
        current_turn(             parent_context.current_turn),
        in_design_id(             parent_context.in_design_id),
        production_block_size(    parent_context.production_block_size),
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


    // disable implicit conversions to CurrentValueVariant
    ScriptingContext(const ScriptingContext&, UniverseObject*, int) = delete;
    ScriptingContext(ScriptingContext&&, UniverseObject*, int) = delete;
    ScriptingContext(const ScriptingContext&, UniverseObject*, double) = delete;
    ScriptingContext(ScriptingContext&&, UniverseObject*, double) = delete;
    ScriptingContext(ScriptingContext&&, UniverseObject*, PlanetType) = delete;
    ScriptingContext(const ScriptingContext&, UniverseObject*, PlanetType) = delete;
    ScriptingContext(ScriptingContext&&, UniverseObject*, PlanetSize) = delete;
    ScriptingContext(const ScriptingContext&, UniverseObject*, PlanetSize) = delete;
    ScriptingContext(ScriptingContext&&, UniverseObject*, ::PlanetEnvironment) = delete;
    ScriptingContext(const ScriptingContext&, UniverseObject*, ::PlanetEnvironment) = delete;
    ScriptingContext(ScriptingContext&&, UniverseObject*, StarType) = delete;
    ScriptingContext(const ScriptingContext&, UniverseObject*, StarType) = delete;
    ScriptingContext(ScriptingContext&&, UniverseObject*, UniverseObjectType) = delete;
    ScriptingContext(const ScriptingContext&, UniverseObject*, UniverseObjectType) = delete;
    ScriptingContext(ScriptingContext&&, UniverseObject*, Visibility) = delete;
    ScriptingContext(const ScriptingContext&, UniverseObject*, Visibility) = delete;
    ScriptingContext(ScriptingContext&&, UniverseObject*, std::string) = delete;
    ScriptingContext(const ScriptingContext&, UniverseObject*, std::string) = delete;
    ScriptingContext(ScriptingContext&&, UniverseObject*, std::vector<std::string>) = delete;
    ScriptingContext(const ScriptingContext&, UniverseObject*, std::vector<std::string>) = delete;

    ScriptingContext(const UniverseObject* source_, UniverseObject* target_) noexcept :
        source(           source_),
        effect_target(    target_),
        galaxy_setup_data(GetGalaxySetupData()),
        species(          GetSpeciesManager()),
        supply(           GetSupplyManager()),
        universe(         &GetUniverse()),
        const_universe(   GetUniverse()),
        empires(          &(::Empires())),
        const_empires(    ::Empires()),
        diplo_statuses(   ::Empires().GetDiplomaticStatuses())
    {}

    ScriptingContext(Universe& universe, EmpireManager& empires_,
                     const GalaxySetupData& galaxy_setup_data_ = GetGalaxySetupData(),
                     SpeciesManager& species_ = GetSpeciesManager(),
                     const SupplyManager& supply_ = GetSupplyManager()) noexcept :
        galaxy_setup_data(galaxy_setup_data_),
        species(          species_),
        supply(           supply_),
        universe(         &universe),
        const_universe(   universe),
        empires(          &empires_),
        const_empires(    empires_),
        diplo_statuses(   empires_.GetDiplomaticStatuses())
    {}

    explicit ScriptingContext(CombatInfo& info, // in CombatSystem.cpp
                              UniverseObject* attacker_as_source = nullptr) noexcept;

    ScriptingContext(const Universe& universe, const EmpireManager& empires_,
                     const UniverseObject* source_ = nullptr,
                     UniverseObject* target_ = nullptr,
                     const CurrentValueVariant& current_value_ = DEFAULT_CURRENT_VALUE) noexcept :
        source(        source_),
        effect_target( target_),
        current_value( current_value_),
        universe(      nullptr),
        const_universe(universe),
        empires(       nullptr),
        const_empires( empires_),
        diplo_statuses(empires_.GetDiplomaticStatuses())
    {}



    // helper functions for accessing state in this context

    // immutable container of immutable objects
    [[nodiscard]] const Universe& ContextUniverse() const noexcept { return const_universe; }

    // mutable container of mutable objects, not thread safe to modify
    [[nodiscard]] Universe& ContextUniverse() {
        if (universe)
            return *universe;
        ErrorLogger() << "ScriptingContext::ContextUniverse() asked for undefined mutable Universe";
        throw std::runtime_error("ScriptingContext::ContextUniverse() asked for undefined mutable Universe");
    }

    // immutable container of immutable objects
    [[nodiscard]] const ObjectMap& ContextObjects() const noexcept { return const_objects; }

    // mutable container of mutable objects, not thread safe to modify
    [[nodiscard]] ObjectMap& ContextObjects() {
        if (objects)
            return *objects;
        ErrorLogger() << "ScriptingContext::ContextUniverse() asked for undefined mutable ObjectMap";
        throw std::runtime_error("ScriptingContext::ContextUniverse() asked for undefined mutable objects");
    }

    [[nodiscard]] DiplomaticStatus ContextDiploStatus(int empire1, int empire2) const {
        if (empire1 == ALL_EMPIRES || empire2 == ALL_EMPIRES || empire1 == empire2)
            return DiplomaticStatus::INVALID_DIPLOMATIC_STATUS;
        const auto high_low_ids = empire1 > empire2 ? std::pair{empire1, empire2} : std::pair{empire2, empire1};
        const auto it = diplo_statuses.find(high_low_ids);
        return it == diplo_statuses.end() ? DiplomaticStatus::INVALID_DIPLOMATIC_STATUS : it->second;
    }

    [[nodiscard]] auto GetEmpireIDsWithDiplomaticStatusWithEmpire(
        int empire_id, DiplomaticStatus diplo_status) const
    {
        return EmpireManager::GetEmpireIDsWithDiplomaticStatusWithEmpire(
            empire_id, diplo_status, diplo_statuses);
    }

    [[nodiscard]] Visibility ContextVis(int object_id, int empire_id) const {
        const auto empire_it = empire_object_vis.find(empire_id);
        if (empire_it == empire_object_vis.end())
            return Visibility::VIS_NO_VISIBILITY;
        const auto object_it = empire_it->second.find(object_id);
        if (object_it == empire_it->second.end())
            return Visibility::VIS_NO_VISIBILITY;
        return object_it->second;
    }

    // mutable empire not thread safe to modify
    [[nodiscard]] std::shared_ptr<Empire> GetEmpire(int id) {
        if (!empires) {
            ErrorLogger() << "ScriptingContext::GetEmpire() asked for unavailable mutable Empire";
            return nullptr;
        }
        return empires->GetEmpire(id);
    }

    [[nodiscard]] std::shared_ptr<const Empire> GetEmpire(int id) const
    { return const_empires.GetEmpire(id); }

    [[nodiscard]] const EmpireManager& Empires() const noexcept
    { return const_empires; } // const container of const empires

    [[nodiscard]] EmpireManager& Empires() { // const container of mutable empires
        if (empires)
            return *empires;
        ErrorLogger() << "ScriptingContext::ContextUniverse() asked for undefined mutable empires";
        throw std::runtime_error("ScriptingContext::ContextUniverse() asked for undefined mutable empires");
    }

    [[nodiscard]] const auto& EmpireIDs() const noexcept
    { return const_empires.EmpireIDs(); }

    // script evaluation local state, some of which may vary during evaluation of an expression
    const UniverseObject*      source = nullptr;
    UniverseObject*            effect_target = nullptr;
    const UniverseObject*      condition_root_candidate = nullptr;
    const UniverseObject*      condition_local_candidate = nullptr;
    const CurrentValueVariant& current_value = DEFAULT_CURRENT_VALUE;

    // general gamestate info
    int                                            combat_bout = 0; // first round of battle is combat_bout == 1
    int                                            current_turn = CurrentTurn();
    int                                            in_design_id = INVALID_DESIGN_ID;
    int                                            production_block_size = 1;
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
    EmpireManager*                                 empires = nullptr;
    const EmpireManager&                           const_empires{empires ? *empires : (::Empires())};
public:
    const EmpireManager::DiploStatusMap&           diplo_statuses{::Empires().GetDiplomaticStatuses()};
};


#endif
