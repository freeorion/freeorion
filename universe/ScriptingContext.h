#ifndef _ScriptingContext_h_
#define _ScriptingContext_h_


#include "Universe.h"
#include "../Empire/EmpireManager.h"
#include "../util/AppInterface.h"
#include <variant>

#if !defined(CONSTEXPR_VEC_AND_STRING)
#  if defined(__cpp_lib_constexpr_vector) && defined(__cpp_lib_constexpr_string) && ((!defined(__GNUC__) || (__GNUC__ > 12) || (__GNUC__ == 12 && __GNUC_MINOR__ >= 2))) && ((!defined(_MSC_VER) || (_MSC_VER >= 1934))) && ((!defined(__clang_major__) || (__clang_major__ >= 17)))
#    define CONSTEXPR_VEC_AND_STRING constexpr
#  else
#    define CONSTEXPR_VEC_AND_STRING
#  endif
#endif

struct CombatInfo;

struct [[nodiscard]] ScriptingContext final {
    using CurrentValueVariant = std::variant<
        int, double, PlanetType, PlanetSize, ::PlanetEnvironment, StarType,
        UniverseObjectType, Visibility, std::string, std::vector<std::string>>;
    inline static CONSTEXPR_VEC_AND_STRING const CurrentValueVariant DEFAULT_CURRENT_VALUE{0};

    // used to disambiguate constructors
    class LocalCandidate final {};
    class Source final {};
    class Target final {};
    class Attacker final {};

    // TODO: = delete copy constructor?

    [[nodiscard]] explicit ScriptingContext(IApp& app) noexcept :
        current_turn(     app.CurrentTurn()),
        galaxy_setup_data(app.GetGalaxySetupData()),
        species(          app.GetSpeciesManager()),
        supply(           app.GetSupplyManager()),
        universe(         &app.GetUniverse()),
        const_universe(   app.GetUniverse()),
        empires(          &app.Empires()),
        const_empires(    app.Empires())
    {}

    [[nodiscard]] ScriptingContext(const ScriptingContext& parent_context,
                                   LocalCandidate, const UniverseObject* condition_local_candidate_) noexcept :
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

    [[nodiscard]] ScriptingContext(const ScriptingContext& parent_context,
                                   const Universe::EmpireObjectVisibilityMap& vis,
                                   const Universe::EmpireObjectVisibilityTurnMap& vis_turns,
                                   Source, const UniverseObject* source_,
                                   Target, UniverseObject* target_) noexcept :
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

    [[nodiscard]] ScriptingContext(const ScriptingContext& parent_context,
                                   const Universe::EmpireObjectVisibilityMap& vis,
                                   const Universe::EmpireObjectVisibilityTurnMap& vis_turns,
                                   Source, const UniverseObject* source_) noexcept :
        ScriptingContext(parent_context, vis, vis_turns, Source{}, source_,
                         Target{}, parent_context.effect_target)
    {}

    [[nodiscard]] ScriptingContext(const ScriptingContext& parent_context,
                                   const Universe::EmpireObjectVisibilityMap& vis,
                                   const Universe::EmpireObjectVisibilityTurnMap& vis_turns) noexcept :
        ScriptingContext(parent_context, vis, vis_turns, Source{}, parent_context.source)
    {}

    [[nodiscard]] ScriptingContext(const ScriptingContext& parent_context,
                                   Source, const UniverseObject* source_) noexcept :
        ScriptingContext(parent_context, parent_context.empire_object_vis,
                         parent_context.empire_object_vis_turns, Source{}, source_)
    {}

    [[nodiscard]] ScriptingContext(const ScriptingContext& parent_context,
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
    template <typename T>
    ScriptingContext(ScriptingContext&&, T) = delete;
    template <typename T>
    ScriptingContext(const ScriptingContext&, T) = delete;

    [[nodiscard]] ScriptingContext(const ScriptingContext& parent_context,
                                   Source, const UniverseObject* source_,
                                   Target, UniverseObject* target_,
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

    [[nodiscard]] ScriptingContext(const ScriptingContext& parent_context,
                                   Source, const UniverseObject* source_,
                                   Target, UniverseObject* target_) noexcept :
        ScriptingContext(parent_context, Source{}, source_, Target{}, target_,
                         parent_context.in_design_id, parent_context.production_block_size)
    {}

    [[nodiscard]] ScriptingContext(const ScriptingContext& parent_context,
                                   Target, UniverseObject* target_,
                                   const CurrentValueVariant& current_value_,
                                   Source, const UniverseObject* source_) noexcept :
        source(                   source_),
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

    [[nodiscard]] ScriptingContext(const ScriptingContext& parent_context,
                                   Target, UniverseObject* target_,
                                   const CurrentValueVariant& current_value_) noexcept :
        ScriptingContext(parent_context, Target{}, target_, current_value_,
                         Source{}, parent_context.source)
    {}

    // disable implicit conversions to CurrentValueVariant
    template <typename T>
    ScriptingContext(const ScriptingContext&, Target, UniverseObject*, T) = delete;
    template <typename T>
    ScriptingContext(const ScriptingContext&, Target, UniverseObject*, T, Source, const UniverseObject*) = delete;
    template <typename T>
    ScriptingContext(ScriptingContext&&, Target, UniverseObject*, T, Source, const UniverseObject*) = delete;
    template <typename T>
    ScriptingContext(ScriptingContext&&, Target, UniverseObject*, T) = delete;

    [[nodiscard]] explicit ScriptingContext(CombatInfo& info, // in CombatSystem.cpp
                                            Attacker = Attacker{}, UniverseObject* attacker_as_source = nullptr) noexcept;

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
    int                                            current_turn;
    int                                            in_design_id = INVALID_DESIGN_ID;
    int                                            production_block_size = 1;
    const GalaxySetupData&                         galaxy_setup_data;
    SpeciesManager&                                species;
    const SupplyManager&                           supply;
private: // Universe and ObjectMap getters select one of these based on constness
    Universe*                                      universe = nullptr;
    const Universe&                                const_universe;
    ObjectMap*                                     objects = universe ? &(universe->Objects()) : nullptr;
    const ObjectMap&                               const_objects{objects ? *objects : const_universe.Objects()};
public:
    const Universe::EmpireObjectVisibilityMap&     empire_object_vis{const_universe.GetEmpireObjectVisibility()};
    const Universe::EmpireObjectVisibilityTurnMap& empire_object_vis_turns{const_universe.GetEmpireObjectVisibilityTurnMap()};
private:
    EmpireManager*                                 empires = nullptr;
    const EmpireManager&                           const_empires;
public:
    const DiploStatusMap&                          diplo_statuses{const_empires.GetDiplomaticStatuses()};
};


#endif
