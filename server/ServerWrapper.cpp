#include "ServerWrapper.h"

#include "ServerApp.h"
#include "UniverseGenerator.h"

#include "../universe/Condition.h"
#include "../universe/ScriptingContext.h"
#include "../universe/Species.h"
#include "../universe/Special.h"
#include "../universe/System.h"
#include "../universe/Planet.h"
#include "../universe/Building.h"
#include "../universe/BuildingType.h"
#include "../universe/Fleet.h"
#include "../universe/FleetPlan.h"
#include "../universe/Ship.h"
#include "../universe/ShipDesign.h"
#include "../universe/Field.h"
#include "../universe/FieldType.h"
#include "../universe/Tech.h"
#include "../universe/Pathfinder.h"
#include "../universe/Universe.h"
#include "../universe/UnlockableItem.h"
#include "../universe/ValueRef.h"

#include "../util/Directories.h"
#include "../util/Logger.h"
#include "../util/Random.h"
#include "../util/i18n.h"
#include "../util/OptionsDB.h"
#include "../util/SitRepEntry.h"

#include "../Empire/Empire.h"
#include "../Empire/EmpireManager.h"

#include "../python/SetWrapper.h"
#include "../python/CommonWrappers.h"

#include <vector>
#include <map>
#include <string>
#include <utility>

#include <boost/python.hpp>
#include <boost/python/list.hpp>
#include <boost/python/tuple.hpp>
#include <boost/python/extract.hpp>
#include <boost/python/stl_iterator.hpp>
#include <boost/date_time/posix_time/time_formatters.hpp>

#ifdef FREEORION_MACOSX
#include <sys/param.h>
#endif

namespace py = boost::python;


// Helper stuff (classes, functions etc.) exposed to the
// server side Python scripts
namespace {
    // Wrapper for getting empire objects
    auto GetAllEmpiresIDs() -> py::list
    {
        const ScriptingContext& context = ServerApp::GetApp()->GetContext();
        py::list empire_list;
        for (const auto id : context.EmpireIDs())
            empire_list.append(id);
        return empire_list;
    }

    // Wrappers for generating sitrep messages
    void GenerateSitRep(int empire_id, const std::string& template_string,
                        const py::dict& py_params, const std::string& icon)
    {
        ScriptingContext& context = ServerApp::GetApp()->GetContext();
        int sitrep_turn = context.current_turn + 1;

        std::vector<std::pair<std::string, std::string>> params;

        if (py_params) {
            params.reserve(len(py_params));
            for (int i = 0; i < len(py_params); i++) {
                std::string k = py::extract<std::string>(py_params.keys()[i]);
                std::string v = py::extract<std::string>(py_params.values()[i]);
                params.emplace_back(std::move(k), std::move(v));
            }
        }

        if (empire_id == ALL_EMPIRES) {
            for (const auto& empire : context.Empires() | range_values) {
                empire->AddSitRepEntry(CreateSitRep(
                    template_string, sitrep_turn, icon, params));  // copy params for each...
            }
        } else {
            auto empire = context.GetEmpire(empire_id);
            if (!empire) {
                ErrorLogger() << "GenerateSitRep: couldn't get empire with ID " << empire_id;
                return;
            }
            empire->AddSitRepEntry(CreateSitRep(template_string, sitrep_turn, icon,
                                                std::move(params)));
        }
    }

    // Wrappers for Species / SpeciesManager class (member) functions
    auto SpeciesDefaultFocus(const std::string& species_name) -> py::object
    {
        const ScriptingContext& context = ServerApp::GetApp()->GetContext();
        const Species* species = context.species.GetSpecies(species_name);
        if (!species) {
            ErrorLogger() << "SpeciesDefaultFocus: couldn't get species " << species_name;
            return py::object("");
        }
        return py::object(species->DefaultFocus());
    }

    auto SpeciesGetPlanetEnvironment(const std::string& species_name, PlanetType planet_type) -> PlanetEnvironment
    {
        const ScriptingContext& context = ServerApp::GetApp()->GetContext();
        const Species* species = context.species.GetSpecies(species_name);
        if (!species) {
            ErrorLogger() << "SpeciesGetPlanetEnvironment: couldn't get species " << species_name;
            return PlanetEnvironment::INVALID_PLANET_ENVIRONMENT;
        }
        return species->GetPlanetEnvironment(planet_type);
    }

    void SpeciesAddHomeworld(const std::string& species_name, int homeworld_id)
    {
        const ScriptingContext& context = ServerApp::GetApp()->GetContext();
        const Species* species = context.species.GetSpecies(species_name);
        if (!species) {
            ErrorLogger() << "SpeciesAddHomeworld: couldn't get species " << species_name;
            return;
        }
        context.species.AddSpeciesHomeworld(species_name, homeworld_id);
    }

    void SpeciesRemoveHomeworld(const std::string& species_name, int homeworld_id)
    {
        const ScriptingContext& context = ServerApp::GetApp()->GetContext();
        const Species* species = context.species.GetSpecies(species_name);
        if (!species) {
            ErrorLogger() << "SpeciesAddHomeworld: couldn't get species " << species_name;
            return;
        }
        context.species.RemoveSpeciesHomeworld(species_name, homeworld_id);
    }

    auto SpeciesCanColonize(const std::string& species_name) -> bool
    {
        const ScriptingContext& context = ServerApp::GetApp()->GetContext();
        const Species* species = context.species.GetSpecies(species_name);
        if (!species) {
            ErrorLogger() << "SpeciesCanColonize: couldn't get species " << species_name;
            return false;
        }
        return species->CanColonize();
    }

    auto GetAllSpecies() -> py::list
    {
        py::list species_list;
        const ScriptingContext& context = ServerApp::GetApp()->GetContext();
        for (const auto& entry : context.species)
            species_list.append(py::object(entry.first));
        return species_list;
    }

    auto GetPlayableSpecies() -> py::list
    {
        py::list species_list;
        const ScriptingContext& context = ServerApp::GetApp()->GetContext();
        SpeciesManager& species_manager = context.species;
        for (auto it = species_manager.playable_begin(); it != species_manager.playable_end(); ++it)
            species_list.append(py::object(it->first)); // TODO: add GetPlayable() and use range for loop here
        return species_list;
    }

    auto GetNativeSpecies() -> py::list
    {
        py::list species_list;
        const ScriptingContext& context = ServerApp::GetApp()->GetContext();
        SpeciesManager& species_manager = context.species;
        for (auto it = species_manager.native_begin(); it != species_manager.native_end(); ++it)
            species_list.append(py::object(it->first));
        return species_list;
    }

    //Checks the condition against many objects at once.
    //Checking many systems is more efficient because for example monster fleet plans
    //typically uses WithinStarLaneJumps to exclude placement near empires.
    auto FilterIDsWithCondition(const Condition::Condition* cond, const py::list& obj_ids) -> py::list
    {
        py::list permitted_ids;

        if (!cond)
            DebugLogger() << "FilterIDsWithCondition passed null condition";

        const ScriptingContext& context = ServerApp::GetApp()->GetContext();

        Condition::ObjectSet objs;
        py::stl_input_iterator<int> end;
        for (py::stl_input_iterator<int> id(obj_ids); id != end; ++id) {
            if (auto obj = context.ContextObjects().getRaw(*id))
                objs.push_back(obj);
            else
                ErrorLogger() << "FilterIDsWithCondition:: Passed an invalid universe object id " << *id;
        }
        if (objs.empty()) {
            ErrorLogger() << "FilterIDsWithCondition:: Couldn't get any valid objects";
            return permitted_ids;
        }

        Condition::ObjectSet permitted_objs;

        // get location condition and evaluate it with the specified universe object
        // if no location condition has been defined, all objects matches
        if (cond && cond->SourceInvariant()) {
            cond->Eval(context, permitted_objs, objs);
        } else {
            permitted_objs = std::move(objs);
        }

        for (auto* obj : permitted_objs)
            permitted_ids.append(obj->ID());

        return permitted_ids;
    }

    // Wrappers for Specials / SpecialManager functions
    auto SpecialSpawnRate(const std::string special_name) -> double
    {
        const Special* special = GetSpecial(special_name);
        if (!special) {
            ErrorLogger() << "SpecialSpawnRate: couldn't get special " << special_name;
            return 0.0;
        }
        return special->SpawnRate();
    }

    auto SpecialSpawnLimit(const std::string special_name) -> int
    {
        const Special* special = GetSpecial(special_name);
        if (!special) {
            ErrorLogger() << "SpecialSpawnLimit: couldn't get special " << special_name;
            return 0;
        }
        return special->SpawnLimit();
    }

    auto SpecialLocations(const std::string special_name, const py::list& object_ids) -> py::list
    {
        // get special and check if it exists
        const Special* special = GetSpecial(special_name);
        if (!special) {
            ErrorLogger() << "SpecialLocation: couldn't get special " << special_name;
            return py::list();
        }

        return FilterIDsWithCondition(special->Location(), object_ids);
    }

    auto SpecialHasLocation(const std::string special_name) -> bool
    {
        // get special and check if it exists
        const Special* special = GetSpecial(special_name);
        if (!special) {
            ErrorLogger() << "SpecialHasLocation: couldn't get special " << special_name;
            return false;
        }
        return special->Location();
    }

    auto GetAllSpecials() -> py::list
    {
        py::list py_specials;
        for (const auto& special_name : SpecialNames())
            py_specials.append(py::object(std::string{special_name}));
        return py_specials;
    }

    // Wrappers for Empire class member functions
    void EmpireSetName(int empire_id, std::string name)
    {
        ScriptingContext& context = ServerApp::GetApp()->GetContext();
        auto empire = context.GetEmpire(empire_id);
        if (!empire) {
            ErrorLogger() << "EmpireSetName: couldn't get empire with ID " << empire_id;
            return;
        }
        empire->SetName(std::move(name));
    }

    auto EmpireSetHomeworld(int empire_id, int planet_id, const std::string& species_name) -> bool
    {
        ScriptingContext& context = ServerApp::GetApp()->GetContext();
        auto empire = context.GetEmpire(empire_id);
        if (!empire) {
            ErrorLogger() << "EmpireSetHomeworld: couldn't get empire with ID " << empire_id;
            return false;
        }
        return SetEmpireHomeworld(empire.get(), planet_id, species_name, context);
    }

    void EmpireUnlockItem(int empire_id, UnlockableItemType item_type,
                          const std::string& item_name)
    {
        ScriptingContext& context = ServerApp::GetApp()->GetContext();

        auto empire = context.GetEmpire(empire_id);
        if (!empire) {
            ErrorLogger() << "EmpireUnlockItem: couldn't get empire with ID " << empire_id;
            return;
        }
        auto item = UnlockableItem{item_type, item_name};
        empire->UnlockItem(item, context.ContextUniverse(), context.current_turn);
    }

    void EmpireAddShipDesign(int empire_id, const std::string& design_name) {
        ScriptingContext& context = ServerApp::GetApp()->GetContext();

        auto empire = context.GetEmpire(empire_id);
        if (!empire) {
            ErrorLogger() << "EmpireAddShipDesign: couldn't get empire with ID " << empire_id;
            return;
        }

        // check if a ship design with ID ship_design_id has been added to the universe
        const ShipDesign* ship_design = context.ContextUniverse().GetGenericShipDesign(design_name);
        if (!ship_design) {
            ErrorLogger() << "EmpireAddShipDesign: no ship design with name " << design_name << " has been added to the universe";
            return;
        }

        context.ContextUniverse().SetEmpireKnowledgeOfShipDesign(ship_design->ID(), empire_id);
        empire->AddShipDesign(ship_design->ID(), context.ContextUniverse());
    }

    void EmpireSetStockpile(int empire_id, ResourceType resource_type, double value) {
        auto empire = ServerApp::GetApp()->GetContext().GetEmpire(empire_id);
        if (!empire) {
            ErrorLogger() << "EmpireSetStockpile: couldn't get empire with ID " << empire_id;
            return;
        }

        try {
            empire->SetResourceStockpile(resource_type, value);
        } catch (...) {
            ErrorLogger() << "EmpireSetStockpile: empire has no resource pool of type " << resource_type;
            return;
        }
    }

    void EmpireSetDiplomacy(int empire1_id, int empire2_id, DiplomaticStatus status)
    { ServerApp::GetApp()->GetContext().Empires().SetDiplomaticStatus(empire1_id, empire2_id, status); }

    // Wrapper for preunlocked items
    auto LoadUnlockableItemList() -> py::list
    {
        py::list py_items;
        for (const auto& item : ServerApp::GetApp()->GetContext().ContextUniverse().InitiallyUnlockedItems())
            py_items.append(py::object(item));
        return py_items;
    }

    // Wrapper for starting buildings
    auto LoadStartingBuildings() -> py::list
    {
        py::list py_items;
        for (auto building : ServerApp::GetApp()->GetContext().ContextUniverse().InitiallyUnlockedBuildings()) {
            if (GetBuildingType(building.name))
                py_items.append(py::object(building));
            else
                ErrorLogger() << "The item " << building.name << " in the starting building list is not a building.";
        }
        return py_items;
    }

    // Wrappers for ship designs and premade ship designs
    auto ShipDesignCreate(const std::string& name, const std::string& description,
                          const std::string& hull, const py::list& py_parts,
                          const std::string& icon, const std::string& model,
                          bool monster) -> bool
    {
        Universe& universe = ServerApp::GetApp()->GetContext().ContextUniverse();
        // Check for empty name
        if (name.empty()) {
            ErrorLogger() << "CreateShipDesign: tried to create ship design without a name";
            return false;
        }

        // check if a ship design with the same name has already been added to the universe
        if (universe.GetGenericShipDesign(name)) {
            ErrorLogger() << "CreateShipDesign: a ship design with the name " << name
            << " has already been added to the universe";
            return false;
        }

        // copy parts list from Python list to C++ vector
        std::vector<std::string> parts;
        for (int i = 0; i < len(py_parts); i++)
            parts.push_back(py::extract<std::string>(py_parts[i]));


        // Create the design and add it to the universe
        try {
            ShipDesign design(std::invalid_argument(""), name, description, BEFORE_FIRST_TURN,
                              ALL_EMPIRES, hull, parts, icon, model, true, monster);

            const auto new_id = universe.InsertShipDesign(design);
            if (new_id == INVALID_DESIGN_ID) {
                ErrorLogger() << "CreateShipDesign: couldn't insert ship design into universe";
                return false;
            }

        } catch (const std::invalid_argument&) {
            ErrorLogger() << "CreateShipDesign: invalid ship design";
            return false;
        }

        return true;
    }

    auto ShipDesignGetPremadeList() -> py::list
    {
        py::list py_ship_designs;
        for (const auto& design : GetPredefinedShipDesignManager().GetOrderedShipDesigns())
            py_ship_designs.append(py::object(design->Name(false)));
        return py::list(py_ship_designs);
    }

    auto ShipDesignGetMonsterList() -> py::list
    {
        py::list py_monster_designs;
        const auto& manager = GetPredefinedShipDesignManager();
        for (const auto& monster : manager.GetOrderedMonsterDesigns())
            py_monster_designs.append(py::object(monster->Name(false)));
        return py::list(py_monster_designs);
    }

    // Wrappers for starting fleet plans
    class FleetPlanWrapper {
    public:
        // name ctors
        FleetPlanWrapper(FleetPlan&& fleet_plan) :
            m_fleet_plan(std::make_shared<FleetPlan>(std::move(fleet_plan)))
        {}

        FleetPlanWrapper(std::string fleet_name, const py::list& py_designs) {
            std::vector<std::string> designs;
            for (int i = 0; i < len(py_designs); i++)
                designs.push_back(py::extract<std::string>(py_designs[i]));
            m_fleet_plan = std::make_shared<FleetPlan>(std::move(fleet_name), designs, false);
        }

        // name accessors
        py::object Name() const
        { return py::object(m_fleet_plan->Name()); }

        py::list ShipDesigns() {
            py::list py_designs;
            for (const auto& design_name : m_fleet_plan->ShipDesigns())
                py_designs.append(py::object(design_name));
            return py::list(py_designs);
        }

        const auto& GetFleetPlan() const noexcept { return *m_fleet_plan; }

    private:
        // Use shared_ptr insead of unique_ptr because boost::python requires a deleter
        std::shared_ptr<const FleetPlan> m_fleet_plan;
    };

    auto LoadFleetPlanList() -> py::list {
        py::list py_fleet_plans;
        const auto fleet_plans = ServerApp::GetApp()->GetContext().ContextUniverse().InitiallyUnlockedFleetPlans();
        for (auto* fleet_plan : fleet_plans)
            if (fleet_plan)
                py_fleet_plans.append(FleetPlanWrapper(FleetPlan(*fleet_plan)));
        return py_fleet_plans;
    }

    // Wrappers for starting monster fleet plans
    class MonsterFleetPlanWrapper {
    public:
        // name ctors
        MonsterFleetPlanWrapper(MonsterFleetPlan&& monster_fleet_plan) :
            m_monster_fleet_plan(std::make_shared<MonsterFleetPlan>(std::move(monster_fleet_plan)))
        {}

        MonsterFleetPlanWrapper(std::string fleet_name, const py::list& py_designs,
                                double spawn_rate, int spawn_limit)
        {
            std::vector<std::string> designs;
            for (int i = 0; i < len(py_designs); i++)
                designs.push_back(py::extract<std::string>(py_designs[i]));

            m_monster_fleet_plan =
                std::make_shared<MonsterFleetPlan>(std::move(fleet_name), designs, spawn_rate,
                                                   spawn_limit, nullptr, false);
        }

        // name accessors
        py::object Name() const
        { return py::object(m_monster_fleet_plan->Name()); }

        py::list ShipDesigns()  const{
            py::list py_designs;
            for (const auto& design_name : m_monster_fleet_plan->ShipDesigns())
                py_designs.append(py::object(design_name));
            return py::list(py_designs);
        }

        double SpawnRate() const noexcept
        { return m_monster_fleet_plan->SpawnRate(); }

        int SpawnLimit() const noexcept
        { return m_monster_fleet_plan->SpawnLimit(); }

        py::list Locations(py::list systems) const
        { return FilterIDsWithCondition(m_monster_fleet_plan->Location(), systems); }

        const MonsterFleetPlan& GetMonsterFleetPlan() const noexcept
        { return *m_monster_fleet_plan; }

    private:
        // Use shared_ptr insead of unique_ptr because boost::python requires a deleter
        std::shared_ptr<const MonsterFleetPlan> m_monster_fleet_plan;
    };

    auto LoadMonsterFleetPlanList() -> py::list
    {
        py::list py_monster_fleet_plans;
        const auto monster_fleet_plans = ServerApp::GetApp()->GetContext().ContextUniverse().MonsterFleetPlans();
        for (auto* fleet_plan : monster_fleet_plans)
            py_monster_fleet_plans.append(MonsterFleetPlanWrapper(MonsterFleetPlan(*fleet_plan)));

        return py_monster_fleet_plans;
    }

    // Wrappers for the various universe object classes member funtions
    // This should provide a more safe and consistent set of server side
    // functions to scripters. All wrapper functions work with object ids, so
    // handling with object references and passing them between the languages is
    // avoided.
    //
    // Wrappers for common UniverseObject class member funtions
    auto GetName(int object_id) -> py::object
    {
        const auto obj = ServerApp::GetApp()->GetContext().ContextObjects().getRaw(object_id);
        if (!obj) {
            ErrorLogger() << "GetName: Couldn't get object with ID " << object_id;
            return py::object("");
        }
        return py::object(obj->Name());
    }

    void SetName(int object_id, std::string name)
    {
        auto obj = ServerApp::GetApp()->GetContext().ContextObjects().getRaw(object_id);
        if (!obj) {
            ErrorLogger() << "RenameUniverseObject: Couldn't get object with ID " << object_id;
            return;
        }
        obj->Rename(std::move(name));
    }

    auto GetX(int object_id) -> double
    {
        const auto obj = ServerApp::GetApp()->GetContext().ContextObjects().getRaw(object_id);
        if (!obj) {
            ErrorLogger() << "GetX: Couldn't get object with ID " << object_id;
            return UniverseObject::INVALID_POSITION;
        }
        return obj->X();
    }

    auto GetY(int object_id) -> double
    {
        const auto obj = ServerApp::GetApp()->GetContext().ContextObjects().getRaw(object_id);
        if (!obj) {
            ErrorLogger() << "GetY: Couldn't get object with ID " << object_id;
            return UniverseObject::INVALID_POSITION;
        }
        return obj->Y();
    }

    auto GetPos(int object_id) -> py::tuple
    {
        const auto obj = ServerApp::GetApp()->GetContext().ContextObjects().getRaw(object_id);
        if (!obj) {
            ErrorLogger() << "GetPos: Couldn't get object with ID " << object_id;
            return py::make_tuple(UniverseObject::INVALID_POSITION,
                                  UniverseObject::INVALID_POSITION);
        }
        return py::make_tuple(obj->X(), obj->Y());
    }

    auto GetOwner(int object_id) -> int
    {
        const auto obj = ServerApp::GetApp()->GetContext().ContextObjects().getRaw(object_id);
        if (!obj) {
            ErrorLogger() << "GetOwner: Couldn't get object with ID " << object_id;
            return ALL_EMPIRES;
        }
        return obj->Owner();
    }

    void AddSpecial(int object_id, std::string special_name) {
        ScriptingContext& context = ServerApp::GetApp()->GetContext();

        // get the universe object and check if it exists
        const auto obj = context.ContextObjects().getRaw(object_id);
        if (!obj) {
            ErrorLogger() << "AddSpecial: Couldn't get object with ID " << object_id;
            return;
        }
        // check if the special exists
        const Special* special = GetSpecial(special_name);
        if (!special) {
            ErrorLogger() << "AddSpecial: couldn't get special " << special_name;
            return;
        }

        float capacity = special->InitialCapacity(object_id, context);

        obj->AddSpecial(std::move(special_name), capacity, context.current_turn);
    }

    void RemoveSpecial(int object_id, const std::string special_name) {
        // get the universe object and check if it exists
        const auto obj = ServerApp::GetApp()->GetContext().ContextObjects().getRaw(object_id);
        if (!obj) {
            ErrorLogger() << "RemoveSpecial: Couldn't get object with ID " << object_id;
            return;
        }
        // check if the special exists
        if (!GetSpecial(special_name)) {
            ErrorLogger() << "RemoveSpecial: couldn't get special " << special_name;
            return;
        }
        obj->RemoveSpecial(special_name);
    }

    auto GetAllObjects() -> py::list
    {
        py::list py_all_objects;
        for (auto oid : ServerApp::GetApp()->GetContext().ContextObjects().allWithIDs() | range_keys)
            py_all_objects.append(oid);
        return py_all_objects;
    }

    auto GetSystems() -> py::list
    {
        py::list py_systems;
        for (const auto* system : ServerApp::GetApp()->GetContext().ContextObjects().allRaw<System>())
            py_systems.append(system->ID());
        return py_systems;
    }

    auto CreateSystem(StarType star_type, const std::string& star_name, double x, double y) -> int
    {
        ScriptingContext& context = ServerApp::GetApp()->GetContext();

        // Check if star type is set to valid value
        if ((star_type == StarType::INVALID_STAR_TYPE) || (star_type == StarType::NUM_STAR_TYPES)) {
            ErrorLogger() << "CreateSystem : Can't create a system with a star of type " << star_type;
            return INVALID_OBJECT_ID;
        }

        // Create system and insert it into the object map
        auto& universe = context.ContextUniverse();
        const int turn = context.current_turn;
        auto system = universe.InsertNew<System>(star_type, star_name, x, y, turn);
        if (!system) {
            ErrorLogger() << "CreateSystem : Attempt to insert system into the object map failed";
            return INVALID_OBJECT_ID;
        }

        return system->SystemID();
    }

    auto CreatePlanet(PlanetSize size, PlanetType planet_type, int system_id,
                      int orbit, const std::string& name) -> int
    {
        ScriptingContext& context = ServerApp::GetApp()->GetContext();

        auto system = context.ContextObjects().getRaw<System>(system_id);

        // Perform some validity checks
        // Check if system with id system_id exists
        if (!system) {
            ErrorLogger() << "CreatePlanet : Couldn't get system with ID " << system_id;
            return INVALID_OBJECT_ID;
        }

        // Check if orbit number is within allowed range
        if ((orbit < 0) || (orbit >= static_cast<int>(system->Orbits()))) {
            ErrorLogger() << "CreatePlanet : There is no orbit " << orbit << " in system " << system_id;
            return INVALID_OBJECT_ID;
        }

        // Check if desired orbit is still empty
        if (system->OrbitOccupied(orbit)) {
            ErrorLogger() << "CreatePlanet : Orbit " << orbit << " of system " << system_id << " already occupied";
            return INVALID_OBJECT_ID;
        }

        // Check if planet size is set to valid value
        if ((size < PlanetSize::SZ_TINY) || (size > PlanetSize::SZ_GASGIANT)) {
            ErrorLogger() << "CreatePlanet : Can't create a planet of size " << size;
            return INVALID_OBJECT_ID;
        }

        // Check if planet type is set to valid value
        if ((planet_type < PlanetType::PT_SWAMP) || (planet_type > PlanetType::PT_GASGIANT)) {
            ErrorLogger() << "CreatePlanet : Can't create a planet of type " << planet_type;
            return INVALID_OBJECT_ID;
        }

        // Check if planet type and size match
        // if type is gas giant, size must be too, same goes for asteroids
        if (((planet_type == PlanetType::PT_GASGIANT) && (size != PlanetSize::SZ_GASGIANT)) ||
            ((planet_type == PlanetType::PT_ASTEROIDS) && (size != PlanetSize::SZ_ASTEROIDS)))
        {
            ErrorLogger() << "CreatePlanet : Planet of type " << planet_type << " can't have size " << size;
            return INVALID_OBJECT_ID;
        }

        // Create planet and insert it into the object map
        auto planet = context.ContextUniverse().InsertNew<Planet>(planet_type, size, context.current_turn);
        if (!planet) {
            ErrorLogger() << "CreateSystem : Attempt to insert planet into the object map failed";
            return INVALID_OBJECT_ID;
        }

        // Add planet to system map
        system->Insert(planet, orbit, context.current_turn, context.ContextObjects());

        // If a name has been specified, set planet name
        if (!(name.empty()))
            planet->Rename(name);

        return planet->ID();
    }

    auto CreateBuilding(const std::string& building_type, int planet_id, int empire_id) -> int
    {
        ScriptingContext& context = ServerApp::GetApp()->GetContext();
        ObjectMap& objects = context.ContextObjects();
        auto planet = objects.getRaw<Planet>(planet_id);
        if (!planet) {
            ErrorLogger() << "CreateBuilding: couldn't get planet with ID " << planet_id;
            return INVALID_OBJECT_ID;
        }

        auto system = objects.getRaw<System>(planet->SystemID());
        if (!system) {
            ErrorLogger() << "CreateBuilding: couldn't get system for planet";
            return INVALID_OBJECT_ID;
        }

        auto empire = context.GetEmpire(empire_id);
        if (!empire) {
            ErrorLogger() << "CreateBuilding: couldn't get empire with ID " << empire_id;
            return INVALID_OBJECT_ID;
        }

        auto building = context.ContextUniverse().InsertNew<Building>(
            empire_id, building_type, empire_id, context.current_turn);
        if (!building) {
            ErrorLogger() << "CreateBuilding: couldn't create building";
            return INVALID_OBJECT_ID;
        }

        system->Insert(building, System::NO_ORBIT, context.current_turn, context.ContextObjects());
        planet->AddBuilding(building->ID());
        building->SetPlanetID(planet_id);
        return building->ID();
    }

    auto CreateFleet(const std::string& name, int system_id, int empire_id, bool aggressive = false) -> int
    {
        ScriptingContext& context = ServerApp::GetApp()->GetContext();

        // Get system and check if it exists
        auto system = context.ContextObjects().getRaw<const System>(system_id);
        if (!system) {
            ErrorLogger() << "CreateFleet: couldn't get system with ID " << system_id;
            return INVALID_OBJECT_ID;
        }

        // Create new fleet at the position of the specified system
        auto fleet = context.ContextUniverse().InsertNew<Fleet>(name, system->X(), system->Y(),
                                                                empire_id, context.current_turn);
        if (!fleet) {
            ErrorLogger() << "CreateFleet: couldn't create new fleet";
            return INVALID_OBJECT_ID;
        }

        // Insert fleet into specified system
        system->Insert(fleet, System::NO_ORBIT, context.current_turn, context.ContextObjects());

        // check if we got a fleet name...
        if (name.empty()) {
            // ...no name has been specified, so we have to generate one using the new fleet id
            fleet->Rename(UserString("OBJ_FLEET") + " " + std::to_string(fleet->ID()));
        }

        fleet->SetAggression(aggressive ? FleetDefaults::FLEET_DEFAULT_ARMED : FleetDefaults::FLEET_DEFAULT_UNARMED);

        // return fleet ID
        return fleet->ID();
    }

    auto CreateShip(const std::string& name, const std::string& design_name,
                    const std::string& species, int fleet_id) -> int
    {
        ScriptingContext& context = ServerApp::GetApp()->GetContext();
        Universe& universe = context.ContextUniverse();
        ObjectMap& objects = context.ContextObjects();

        // check if we got a species name, if yes, check if species exists
        if (!species.empty() && !context.species.GetSpecies(species)) {
            ErrorLogger() << "CreateShip: invalid species specified";
            return INVALID_OBJECT_ID;
        }

        // get ship design and check if it exists
        const ShipDesign* ship_design = universe.GetGenericShipDesign(design_name);
        if (!ship_design) {
            ErrorLogger() << "CreateShip: couldn't get ship design " << design_name;
            return INVALID_OBJECT_ID;
        }

        // get fleet and check if it exists
        auto fleet = objects.getRaw<const Fleet>(fleet_id);
        if (!fleet) {
            ErrorLogger() << "CreateShip: couldn't get fleet with ID " << fleet_id;
            return INVALID_OBJECT_ID;
        }

        auto system = objects.getRaw<const System>(fleet->SystemID());
        if (!system) {
            ErrorLogger() << "CreateShip: couldn't get system for fleet";
            return INVALID_OBJECT_ID;
        }

        // get owner empire of specified fleet
        int empire_id = fleet->Owner();
        // if we got the id of an actual empire, get the empire object and check if it exists
        std::shared_ptr<Empire> empire;
        if (empire_id != ALL_EMPIRES) {
            empire = context.GetEmpire(empire_id);
            if (!empire) {
                ErrorLogger() << "CreateShip: couldn't get empire with ID " << empire_id;
                return INVALID_OBJECT_ID;
            }
        }

        // create new ship
        auto ship = universe.InsertNew<Ship>(empire_id, ship_design->ID(), species, universe,
                                             context.species, empire_id, context.current_turn);
        if (!ship) {
            ErrorLogger() << "CreateShip: couldn't create new ship";
            return INVALID_OBJECT_ID;
        }
        system->Insert(ship, System::NO_ORBIT, context.current_turn, objects);

        // set ship name
        // check if we got a ship name...
        if (name.empty()) {
            // ...no name has been specified, so we have to generate one
            // check if the owner empire we got earlier is actually an empire...
            if (empire) {
                // ...yes, so construct a name using the empires NewShipName member function
                ship->Rename(empire->NewShipName());
            } else {
                // ...no, so construct a name using the new ships id
                ship->Rename(UserString("OBJ_SHIP") + " " + std::to_string(ship->ID()));
            }
        } else {
            // ...yes, name has been specified, so use it
            ship->Rename(name);
        }

        // add ship to fleet, this also moves the ship to the
        // fleets location and inserts it into the system
        fleet->AddShips({ship->ID()});
        ship->SetFleetID(fleet->ID());

        // set the meters of the ship to max values
        ship->ResetTargetMaxUnpairedMeters();
        ship->ResetPairedActiveMeters();
        ship->SetShipMetersToMax();

        ship->BackPropagateMeters();

        //return the new ships id
        return ship->ID();
    }

    auto CreateFieldImpl(const std::string& field_type_name, double x, double y, double size,
                         ScriptingContext& context) -> std::shared_ptr<Field>
    {
        // check if a field type with the specified field type name exists and get the field type
        const FieldType* field_type = GetFieldType(field_type_name);
        if (!field_type) {
            ErrorLogger() << "CreateFieldImpl: couldn't get field type with name: " << field_type_name;
            return nullptr;
        }

        // check if the specified size is within sane limits, and reset its value if not
        if (size < 1.0) {
            ErrorLogger() << "CreateFieldImpl given very small / negative size: " << size << ", resetting to 1.0";
            size = 1.0;
        }
        if (size > 10000.0) {
            ErrorLogger() << "CreateFieldImpl given very large size: " << size << ", so resetting to 10000.0";
            size = 10000.0;
        }

        // create the new field
        auto field = context.ContextUniverse().InsertNew<Field>(field_type_name, x, y, size, context.current_turn);
        if (!field) {
            ErrorLogger() << "CreateFieldImpl: couldn't create field";
            return nullptr;
        }

        // get the localized version of the field type name and set that as the fields name
        field->Rename(UserString(field_type->Name()));
        return field;
    }

    auto CreateField(const std::string& field_type_name, double x, double y, double size) -> int
    {
        if (auto field = CreateFieldImpl(field_type_name, x, y, size, ServerApp::GetApp()->GetContext()))
            return field->ID();
        else
            return INVALID_OBJECT_ID;
    }

    auto CreateFieldInSystem(const std::string& field_type_name, double size, int system_id) -> int
    {
        auto& context = ServerApp::GetApp()->GetContext();

        // check if system exists and get system
        auto system = context.ContextObjects().getRaw<const System>(system_id);
        if (!system) {
            ErrorLogger() << "CreateFieldInSystem: couldn't get system with ID " << system_id;
            return INVALID_OBJECT_ID;
        }
        // create the field with the coordinates of the system
        auto field = CreateFieldImpl(field_type_name, system->X(), system->Y(), size, context);
        if (!field)
            return INVALID_OBJECT_ID;
        int field_id = field->ID();
        system->Insert(std::move(field), System::NO_ORBIT, context.current_turn, context.ContextObjects());
        return field_id;
    }

    // Return a list of system ids of universe objects with @p obj_ids.
    auto ObjectsGetSystems(const py::list& obj_ids) -> py::list
    {
        const auto& objs = ServerApp::GetApp()->GetContext().ContextObjects();
        py::list py_systems;
        py::stl_input_iterator<int> end;
        for (py::stl_input_iterator<int> id(obj_ids);
             id != end; ++id) {
            if (auto obj = objs.getRaw(*id)) {
                py_systems.append(obj->SystemID());
            } else {
                ErrorLogger() << "Passed an invalid universe object id " << *id;
                py_systems.append(INVALID_OBJECT_ID);
            }
        }
        return py_systems;
    }

    // Return all systems within \p jumps of \p sys_ids
    auto SystemsWithinJumps(std::size_t jumps, const py::list& sys_ids) -> py::list
    {
        py::list py_systems;
        py::stl_input_iterator<int> end;

        std::vector<int> systems{py::stl_input_iterator<int>(sys_ids), end};
        auto systems_in_vicinity = ServerApp::GetApp()->GetContext().ContextUniverse().GetPathfinder().WithinJumps(jumps, std::move(systems));

        TraceLogger() << "within " << jumps << " jumps: " << systems_in_vicinity.size() << " systems";

        for (auto system_id : systems_in_vicinity)
            py_systems.append(system_id);

        return py_systems;
    }

    // Wrappers for System class member functions
    auto SystemGetStarType(int system_id) -> StarType
    {
        auto system = ServerApp::GetApp()->GetContext().ContextObjects().getRaw<const System>(system_id);
        if (!system) {
            ErrorLogger() << "SystemGetStarType: couldn't get system with ID " << system_id;
            return StarType::INVALID_STAR_TYPE;
        }
        return system->GetStarType();
    }

    void SystemSetStarType(int system_id, StarType star_type) {
        // Check if star type is set to valid value
        if ((star_type == StarType::INVALID_STAR_TYPE) || (star_type == StarType::NUM_STAR_TYPES)) {
            ErrorLogger() << "SystemSetStarType : Can't create a system with a star of type " << star_type;
            return;
        }

        auto system = ServerApp::GetApp()->GetContext().ContextObjects().getRaw<System>(system_id);
        if (!system) {
            ErrorLogger() << "SystemSetStarType : Couldn't get system with ID " << system_id;
            return;
        }

        system->SetStarType(star_type);
    }

    auto SystemGetNumOrbits(int system_id) -> int
    {
        auto system = ServerApp::GetApp()->GetContext().ContextObjects().getRaw<const System>(system_id);
        if (!system) {
            ErrorLogger() << "SystemGetNumOrbits : Couldn't get system with ID " << system_id;
            return 0;
        }
        return system->Orbits();
    }

    auto SystemFreeOrbits(int system_id) -> py::list
    {
        py::list py_orbits;
        auto system = ServerApp::GetApp()->GetContext().ContextObjects().getRaw<const System>(system_id);
        if (!system) {
            ErrorLogger() << "SystemFreeOrbits : Couldn't get system with ID " << system_id;
            return py_orbits;
        }
        for (int orbit_idx : system->FreeOrbits())
            py_orbits.append(orbit_idx);
        return py_orbits;
    }

    auto SystemOrbitOccupied(int system_id, int orbit) -> bool
    {
        auto system = ServerApp::GetApp()->GetContext().ContextObjects().getRaw<const System>(system_id);
        if (!system) {
            ErrorLogger() << "SystemOrbitOccupied : Couldn't get system with ID " << system_id;
            return 0;
        }
        return system->OrbitOccupied(orbit);
    }

    auto SystemOrbitOfPlanet(int system_id, int planet_id) -> int
    {
        auto system = ServerApp::GetApp()->GetContext().ContextObjects().getRaw<const System>(system_id);
        if (!system) {
            ErrorLogger() << "SystemOrbitOfPlanet : Couldn't get system with ID " << system_id;
            return 0;
        }
        return system->OrbitOfPlanet(planet_id);
    }

    auto SystemGetPlanets(int system_id) -> py::list
    {
        py::list py_planets;
        auto system = ServerApp::GetApp()->GetContext().ContextObjects().getRaw<const System>(system_id);
        if (!system) {
            ErrorLogger() << "SystemGetPlanets : Couldn't get system with ID " << system_id;
            return py_planets;
        }
        for (int planet_id : system->PlanetIDs())
            py_planets.append(planet_id);
        return py_planets;
    }

    auto SystemGetFleets(int system_id) -> py::list
    {
        py::list py_fleets;
        auto system = ServerApp::GetApp()->GetContext().ContextObjects().getRaw<const System>(system_id);
        if (!system) {
            ErrorLogger() << "SystemGetFleets : Couldn't get system with ID " << system_id;
            return py_fleets;
        }
        for (int fleet_id : system->FleetIDs())
            py_fleets.append(fleet_id);
        return py_fleets;
    }

    auto SystemGetStarlanes(int system_id) -> py::list
    {
        py::list py_starlanes;
        // get source system
        auto system = ServerApp::GetApp()->GetContext().ContextObjects().getRaw<const System>(system_id);
        if (!system) {
            ErrorLogger() << "SystemGetStarlanes : Couldn't get system with ID " << system_id;
            return py_starlanes;
        }
        // get list of systems the source system has starlanes to
        for (const auto lane_to_id : system->Starlanes())
            py_starlanes.append(lane_to_id);
        return py_starlanes;
    }

    void SystemAddStarlane(int from_sys_id, int to_sys_id)
    {
        auto& objs = ServerApp::GetApp()->GetContext().ContextObjects();
        // get source and destination system, check that both exist
        auto from_sys = objs.getRaw<System>(from_sys_id);
        if (!from_sys) {
            ErrorLogger() << "SystemAddStarlane : Couldn't find system with ID " << from_sys_id;
            return;
        }
        auto to_sys = objs.getRaw<System>(to_sys_id);
        if (!to_sys) {
            ErrorLogger() << "SystemAddStarlane : Couldn't find system with ID " << to_sys_id;
            return;
        }
        // add the starlane on both ends
        from_sys->AddStarlane(to_sys_id);
        to_sys->AddStarlane(from_sys_id);
    }

    void SystemRemoveStarlane(int from_sys_id, int to_sys_id)
    {
        auto& objs = ServerApp::GetApp()->GetContext().ContextObjects();
        // get source and destination system, check that both exist
        auto from_sys = objs.getRaw<System>(from_sys_id);
        if (!from_sys) {
            ErrorLogger() << "SystemRemoveStarlane : Couldn't find system with ID " << from_sys_id;
            return;
        }
        auto to_sys = objs.getRaw<System>(to_sys_id);
        if (!to_sys) {
            ErrorLogger() << "SystemRemoveStarlane : Couldn't find system with ID " << to_sys_id;
            return;
        }
        // remove the starlane from both ends
        from_sys->RemoveStarlane(to_sys_id);
        to_sys->RemoveStarlane(from_sys_id);
    }

    // Wrapper for Planet class member functions
    auto PlanetGetType(int planet_id) -> PlanetType
    {
        auto planet = ServerApp::GetApp()->GetContext().ContextObjects().getRaw<const Planet>(planet_id);
        if (!planet) {
            ErrorLogger() << "PlanetGetType: Couldn't get planet with ID " << planet_id;
            return PlanetType::INVALID_PLANET_TYPE;
        }
        return planet->Type();
    }

    void PlanetSetType(int planet_id, PlanetType planet_type)
    {
        auto planet = ServerApp::GetApp()->GetContext().ContextObjects().getRaw<Planet>(planet_id);
        if (!planet) {
            ErrorLogger() << "PlanetSetType: Couldn't get planet with ID " << planet_id;
            return;
        }

        planet->SetType(planet_type);
        if (planet_type == PlanetType::PT_ASTEROIDS)
            planet->SetSize(PlanetSize::SZ_ASTEROIDS);
        else if (planet_type == PlanetType::PT_GASGIANT)
            planet->SetSize(PlanetSize::SZ_GASGIANT);
        else if (planet->Size() == PlanetSize::SZ_ASTEROIDS)
            planet->SetSize(PlanetSize::SZ_TINY);
        else if (planet->Size() == PlanetSize::SZ_GASGIANT)
            planet->SetSize(PlanetSize::SZ_HUGE);
    }

    auto PlanetGetSize(int planet_id) -> PlanetSize
    {
        auto planet = ServerApp::GetApp()->GetContext().ContextObjects().getRaw<const Planet>(planet_id);
        if (!planet) {
            ErrorLogger() << "PlanetGetSize: Couldn't get planet with ID " << planet_id;
            return PlanetSize::INVALID_PLANET_SIZE;
        }
        return planet->Size();
    }

    void PlanetSetSize(int planet_id, PlanetSize planet_size)
    {
        auto planet = ServerApp::GetApp()->GetContext().ContextObjects().getRaw<Planet>(planet_id);
        if (!planet) {
            ErrorLogger() << "PlanetSetSize: Couldn't get planet with ID " << planet_id;
            return;
        }

        planet->SetSize(planet_size);
        if (planet_size == PlanetSize::SZ_ASTEROIDS)
            planet->SetType(PlanetType::PT_ASTEROIDS);
        else if (planet_size == PlanetSize::SZ_GASGIANT)
            planet->SetType(PlanetType::PT_GASGIANT);
        else if ((planet->Type() == PlanetType::PT_ASTEROIDS) || (planet->Type() == PlanetType::PT_GASGIANT))
            planet->SetType(PlanetType::PT_BARREN);
    }

    auto PlanetGetSpecies(int planet_id) -> py::object
    {
        auto planet = ServerApp::GetApp()->GetContext().ContextObjects().getRaw<const Planet>(planet_id);
        if (!planet) {
            ErrorLogger() << "PlanetGetSpecies: Couldn't get planet with ID " << planet_id;
            return py::object("");
        }
        return py::object(planet->SpeciesName());
    }

    void PlanetSetSpecies(int planet_id, const std::string& species_name)
    {
        auto& context = ServerApp::GetApp()->GetContext();
        auto planet = context.ContextObjects().getRaw<Planet>(planet_id);
        if (!planet) {
            ErrorLogger() << "PlanetSetSpecies: Couldn't get planet with ID " << planet_id;
            return;
        }
        planet->SetSpecies(species_name, context.current_turn, context.species);
    }

    auto PlanetGetFocus(int planet_id) -> py::object
    {
        auto planet = ServerApp::GetApp()->GetContext().ContextObjects().getRaw<const Planet>(planet_id);
        if (!planet) {
            ErrorLogger() << "PlanetGetFocus: Couldn't get planet with ID " << planet_id;
            return py::object("");
        }
        return py::object(planet->Focus());
    }

    void PlanetSetFocus(int planet_id, const std::string& focus)
    {
        auto& context = ServerApp::GetApp()->GetContext();
        auto planet = context.ContextObjects().getRaw<Planet>(planet_id);
        if (!planet) {
            ErrorLogger() << "PlanetSetSpecies: Couldn't get planet with ID " << planet_id;
            return;
        }
        planet->SetFocus(focus, context);
    }

    auto PlanetAvailableFoci(int planet_id) -> py::list
    {
        auto& context = ServerApp::GetApp()->GetContext();
        py::list py_foci;
        auto planet = context.ContextObjects().getRaw<const Planet>(planet_id);
        if (!planet) {
            ErrorLogger() << "PlanetAvailableFoci: Couldn't get planet with ID " << planet_id;
            return py_foci;
        }
        for (const auto& focus : planet->AvailableFoci(context))
            py_foci.append(py::object(std::string{focus}));

        return py_foci;
    }

    auto PlanetMakeOutpost(int planet_id, int empire_id) -> bool
    {
        auto& context = ServerApp::GetApp()->GetContext();

        auto planet = context.ContextObjects().getRaw<Planet>(planet_id);
        if (!planet) {
            ErrorLogger() << "PlanetMakeOutpost: couldn't get planet with ID:" << planet_id;
            return false;
        }

        if (!context.GetEmpire(empire_id)) {
            ErrorLogger() << "PlanetMakeOutpost: couldn't get empire with ID " << empire_id;
            return false;
        }

        return planet->Colonize(empire_id, "", 0.0, context);
    }

    auto PlanetMakeColony(int planet_id, int empire_id, const std::string& species, double population) -> bool
    {
        auto& context = ServerApp::GetApp()->GetContext();

        auto planet = context.ContextObjects().get<Planet>(planet_id);
        if (!planet) {
            ErrorLogger() << "PlanetMakeColony: couldn't get planet with ID:" << planet_id;
            return false;
        }

        if (!context.GetEmpire(empire_id)) {
            ErrorLogger() << "PlanetMakeColony: couldn't get empire with ID " << empire_id;
            return false;
        }

        if (!context.species.GetSpecies(species)) {
            ErrorLogger() << "PlanetMakeColony: couldn't get species with name: " << species;
            return false;
        }

        if (population < 0.0)
            population = 0.0;

        return planet->Colonize(empire_id, species, population, context);
    }

    auto PlanetCardinalSuffix(int planet_id) -> py::object
    {
        const auto& context = ServerApp::GetApp()->GetContext();

        auto planet = context.ContextObjects().get<Planet>(planet_id);
        if (!planet) {
            ErrorLogger() << "PlanetCardinalSuffix: couldn't get planet with ID:" << planet_id;
            return py::object(UserString("ERROR"));
        }

        return py::object(planet->CardinalSuffix(context.ContextObjects()));
    }

    auto PlayerEmpireColor(const PlayerSetupData* psd) -> py::tuple
    {
        EmpireColor color = psd->empire_color;
        return py::make_tuple(
            std::get<0>(color),
            std::get<1>(color),
            std::get<2>(color),
            std::get<3>(color));
    }
}

namespace FreeOrionPython {
    void WrapServer() {
        py::class_<PlayerSetupData>("PlayerSetupData")
            .def_readwrite("player_name",        &PlayerSetupData::player_name)
            .def_readwrite("empire_name",        &PlayerSetupData::empire_name)
            .add_property("empire_color",        PlayerEmpireColor)
            .def_readwrite("starting_species",   &PlayerSetupData::starting_species_name)
            .def_readwrite("starting_team",      &PlayerSetupData::starting_team);

        py::class_<FleetPlanWrapper>("FleetPlan", py::init<const std::string&, const py::list&>())
            .def("name",                        &FleetPlanWrapper::Name)
            .def("ship_designs",                &FleetPlanWrapper::ShipDesigns);

        py::class_<MonsterFleetPlanWrapper>("MonsterFleetPlan", py::init<const std::string&, const py::list&, double, int>())
            .def("name",                        &MonsterFleetPlanWrapper::Name)
            .def("ship_designs",                &MonsterFleetPlanWrapper::ShipDesigns)
            .def("spawn_rate",                  &MonsterFleetPlanWrapper::SpawnRate)
            .def("spawn_limit",                 &MonsterFleetPlanWrapper::SpawnLimit)
            .def("locations",                   &MonsterFleetPlanWrapper::Locations);

        py::def("get_universe",
                +[]() -> const Universe& { return ServerApp::GetApp()->GetUniverse(); },
                py::return_value_policy<py::reference_existing_object>());
        py::def("get_all_empires",              GetAllEmpiresIDs);
        py::def("get_empire",
                +[](int id) -> const Empire* { return ServerApp::GetApp()->GetEmpire(id); },
                py::return_value_policy<py::reference_existing_object>());

        py::def("userString",
                +[](const std::string& key) -> const std::string& { return UserString(key); },
                py::return_value_policy<py::copy_const_reference>());
        py::def("userStringExists",
                +[](const std::string& key) -> bool { return UserStringExists(key); });
        //py::def("userStringList",               &GetUserStringList); // could be copied from AIWrapper

        py::def("roman_number",                     RomanNumber);
        py::def("get_resource_dir",                 +[]() -> py::object { return py::object(PathToString(GetResourceDir())); });

        py::def("all_empires",                      +[]() -> int { return ALL_EMPIRES; });
        py::def("invalid_object",                   +[]() -> int { return INVALID_OBJECT_ID; });
        py::def("large_meter_value",                +[]() -> float { return Meter::LARGE_VALUE; });
        py::def("invalid_position",                 +[]() -> double { return UniverseObject::INVALID_POSITION; });

        py::def("get_galaxy_setup_data",            GetGalaxySetupData,             py::return_value_policy<py::reference_existing_object>());
        py::def("current_turn",                     +[]() -> int { return ServerApp::GetApp()->GetContext().current_turn; });
        py::def("generate_sitrep",                  GenerateSitRep);
        py::def("generate_sitrep",                  +[](int empire_id, const std::string& template_string, const std::string& icon) { GenerateSitRep(empire_id, template_string, py::dict(), icon); });
        py::def("generate_starlanes",               +[](int max_jumps_between_systems, int max_starlane_length) { auto& context = ServerApp::GetApp()->GetContext(); GenerateStarlanes(max_jumps_between_systems, max_starlane_length, context.ContextUniverse(), context.Empires()); });

        py::def("species_preferred_focus",          SpeciesDefaultFocus);
        py::def("species_get_planet_environment",   SpeciesGetPlanetEnvironment);
        py::def("species_add_homeworld",            SpeciesAddHomeworld);
        py::def("species_remove_homeworld",         SpeciesRemoveHomeworld);
        py::def("species_can_colonize",             SpeciesCanColonize);
        py::def("get_all_species",                  GetAllSpecies);
        py::def("get_playable_species",             GetPlayableSpecies);
        py::def("get_native_species",               GetNativeSpecies);

        py::def("special_spawn_rate",               SpecialSpawnRate);
        py::def("special_spawn_limit",              SpecialSpawnLimit);
        py::def("special_locations",                SpecialLocations);
        py::def("special_has_location",             SpecialHasLocation);
        py::def("get_all_specials",                 GetAllSpecials);

        py::def("empire_set_name",                  EmpireSetName);
        py::def("empire_set_homeworld",             EmpireSetHomeworld);
        py::def("empire_unlock_item",               EmpireUnlockItem);
        py::def("empire_add_ship_design",           EmpireAddShipDesign);
        py::def("empire_set_stockpile",             EmpireSetStockpile);
        py::def("empire_set_diplomacy",             EmpireSetDiplomacy);

        py::def("design_create",                    ShipDesignCreate);
        py::def("design_get_premade_list",          ShipDesignGetPremadeList);
        py::def("design_get_monster_list",          ShipDesignGetMonsterList);

        py::def("load_unlockable_item_list",        LoadUnlockableItemList);
        py::def("load_starting_buildings",          LoadStartingBuildings);
        py::def("load_fleet_plan_list",             LoadFleetPlanList);
        py::def("load_monster_fleet_plan_list",     LoadMonsterFleetPlanList);

        py::def("get_name",                         GetName, "Returns the name (string) of the universe object with the specified object id (int). If there is no such object, returns an empty string and logs an error to the error log.");
        py::def("set_name",                         SetName, "Sets the name (string) of the universe object with the specified object id (int). If there is no such object, just logs an error to the error log.");
        py::def("get_x",                            GetX);
        py::def("get_y",                            GetY);
        py::def("get_pos",                          GetPos);
        py::def("get_owner",                        GetOwner);
        py::def("add_special",                      AddSpecial);
        py::def("remove_special",                   RemoveSpecial);

        py::def("get_universe_width",               +[]() -> double { return ServerApp::GetApp()->GetContext().ContextUniverse().UniverseWidth(); });
        py::def("set_universe_width",               +[](double width) { ServerApp::GetApp()->GetContext().ContextUniverse().SetUniverseWidth(width); });
        py::def("linear_distance",                  +[](int system1_id, int system2_id) -> double { const auto& context = ServerApp::GetApp()->GetContext(); return context.ContextUniverse().GetPathfinder().LinearDistance(system1_id, system2_id, context.ContextObjects()); });
        py::def("jump_distance",                    +[](int system1_id, int system2_id) -> int { return ServerApp::GetApp()->GetContext().ContextUniverse().GetPathfinder().JumpDistanceBetweenSystems(system1_id, system2_id); });
        py::def("get_all_objects",                  GetAllObjects);
        py::def("get_systems",                      GetSystems);
        py::def("create_system",                    CreateSystem);
        py::def("create_planet",                    CreatePlanet);
        py::def("create_building",                  CreateBuilding);
        py::def("create_fleet",                     CreateFleet);
        py::def("create_ship",                      CreateShip);
        py::def("create_monster_fleet",             +[](int system_id) -> int { return CreateFleet(UserString("MONSTERS"), system_id, ALL_EMPIRES, true); });
        py::def("create_monster",                   +[](const std::string& design_name, int fleet_id) -> int { return CreateShip(NewMonsterName(), design_name, "", fleet_id); });
        py::def("create_field",                     CreateField);
        py::def("create_field_in_system",           CreateFieldInSystem);

        py::def("objs_get_systems",                 ObjectsGetSystems);

        py::def("systems_within_jumps_unordered",   SystemsWithinJumps, "Return all systems within ''jumps'' of the systems with ids ''sys_ids''");

        py::def("sys_get_star_type",                SystemGetStarType);
        py::def("sys_set_star_type",                SystemSetStarType);
        py::def("sys_get_num_orbits",               SystemGetNumOrbits);
        py::def("sys_free_orbits",                  SystemFreeOrbits);
        py::def("sys_orbit_occupied",               SystemOrbitOccupied);
        py::def("sys_orbit_of_planet",              SystemOrbitOfPlanet);
        py::def("sys_get_planets",                  SystemGetPlanets);
        py::def("sys_get_fleets",                   SystemGetFleets);
        py::def("sys_get_starlanes",                SystemGetStarlanes);
        py::def("sys_add_starlane",                 SystemAddStarlane);
        py::def("sys_remove_starlane",              SystemRemoveStarlane);

        py::def("planet_get_type",                  PlanetGetType);
        py::def("planet_set_type",                  PlanetSetType);
        py::def("planet_get_size",                  PlanetGetSize);
        py::def("planet_set_size",                  PlanetSetSize);
        py::def("planet_get_species",               PlanetGetSpecies);
        py::def("planet_set_species",               PlanetSetSpecies);
        py::def("planet_get_focus",                 PlanetGetFocus);
        py::def("planet_set_focus",                 PlanetSetFocus);
        py::def("planet_available_foci",            PlanetAvailableFoci);
        py::def("planet_make_outpost",              PlanetMakeOutpost);
        py::def("planet_make_colony",               PlanetMakeColony);
        py::def("planet_cardinal_suffix",           PlanetCardinalSuffix);
    }
}
