#include "ServerWrapper.h"

#include "../../universe/Condition.h"
#include "../../universe/Species.h"
#include "../../universe/Special.h"
#include "../../universe/System.h"
#include "../../universe/Planet.h"
#include "../../universe/Building.h"
#include "../../universe/Fleet.h"
#include "../../universe/Ship.h"
#include "../../universe/Field.h"
#include "../../universe/Tech.h"
#include "../../universe/Pathfinder.h"
#include "../../universe/Universe.h"
#include "../../universe/UniverseGenerator.h"
#include "../../universe/Enums.h"
#include "../../universe/ValueRef.h"

#include "../../server/ServerApp.h"
#include "../../util/Directories.h"
#include "../../util/Logger.h"
#include "../../util/Random.h"
#include "../../util/i18n.h"
#include "../../util/OptionsDB.h"
#include "../../util/SitRepEntry.h"

#include "../../Empire/Empire.h"
#include "../../Empire/EmpireManager.h"

#include "../SetWrapper.h"
#include "../CommonWrappers.h"

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

using boost::python::class_;
using boost::python::def;
using boost::python::init;
using boost::python::no_init;
using boost::noncopyable;
using boost::python::return_value_policy;
using boost::python::copy_const_reference;
using boost::python::reference_existing_object;
using boost::python::return_by_value;
using boost::python::return_internal_reference;

using boost::python::object;
using boost::python::import;
using boost::python::error_already_set;
using boost::python::exec;
using boost::python::dict;
using boost::python::list;
using boost::python::tuple;
using boost::python::make_tuple;
using boost::python::extract;
using boost::python::len;
using boost::python::long_;


FO_COMMON_API extern const int INVALID_DESIGN_ID;

// Helper stuff (classes, functions etc.) exposed to the
// server side Python scripts
namespace {
    // Functions that return various important constants
    int     AllEmpires()
    { return ALL_EMPIRES; }

    int     InvalidObjectID()
    { return INVALID_OBJECT_ID; }

    float   LargeMeterValue()
    { return Meter::LARGE_VALUE; }

    double  InvalidPosition()
    { return UniverseObject::INVALID_POSITION; }

    // Wrapper for GetResourceDir
    object GetResourceDirWrapper()
    { return object(PathToString(GetResourceDir())); }

    // Wrapper for GetUserConfigDir
    object GetUserConfigDirWrapper()
    { return object(PathToString(GetUserConfigDir())); }

    // Wrapper for getting empire objects
    list GetAllEmpires() {
        list empire_list;
        for (const auto& entry : Empires())
            empire_list.append(entry.second->EmpireID());
        return empire_list;
    }

    // Wrappers for generating sitrep messages
    void GenerateSitRep(int empire_id, const std::string& template_string,
                        const dict& py_params, const std::string& icon)
    {
        int sitrep_turn = CurrentTurn() + 1;

        std::vector<std::pair<std::string, std::string>> params;

        if (py_params) {
            for (int i = 0; i < len(py_params); i++) {
                std::string k = extract<std::string>(py_params.keys()[i]);
                std::string v = extract<std::string>(py_params.values()[i]);
                params.push_back({k, v});
            }
        }

        if (empire_id == ALL_EMPIRES) {
            for (const auto& entry : Empires()) {
                entry.second->AddSitRepEntry(CreateSitRep(template_string, sitrep_turn,
                                                          icon, params));
            }
        } else {
            Empire* empire = GetEmpire(empire_id);
            if (!empire) {
                ErrorLogger() << "GenerateSitRep: couldn't get empire with ID " << empire_id;
                return;
            }
            empire->AddSitRepEntry(CreateSitRep(template_string, sitrep_turn, icon, params));
        }
    }

    void GenerateSitRep1(int empire_id,
                         const std::string& template_string,
                         const std::string& icon)
    { GenerateSitRep(empire_id, template_string, dict(), icon); }

    // Wrappers for Species / SpeciesManager class (member) functions
    object SpeciesPreferredFocus(const std::string& species_name) {
        const Species* species = GetSpecies(species_name);
        if (!species) {
            ErrorLogger() << "SpeciesPreferredFocus: couldn't get species " << species_name;
            return object("");
        }
        return object(species->PreferredFocus());
    }

    PlanetEnvironment SpeciesGetPlanetEnvironment(const std::string& species_name, PlanetType planet_type) {
        const Species* species = GetSpecies(species_name);
        if (!species) {
            ErrorLogger() << "SpeciesGetPlanetEnvironment: couldn't get species " << species_name;
            return INVALID_PLANET_ENVIRONMENT;
        }
        return species->GetPlanetEnvironment(planet_type);
    }

    void SpeciesAddHomeworld(const std::string& species_name, int homeworld_id) {
        Species* species = SpeciesManager::GetSpeciesManager().GetSpecies(species_name);
        if (!species) {
            ErrorLogger() << "SpeciesAddHomeworld: couldn't get species " << species_name;
            return;
        }
        species->AddHomeworld(homeworld_id);
    }

    void SpeciesRemoveHomeworld(const std::string& species_name, int homeworld_id) {
        Species* species = SpeciesManager::GetSpeciesManager().GetSpecies(species_name);
        if (!species) {
            ErrorLogger() << "SpeciesAddHomeworld: couldn't get species " << species_name;
            return;
        }
        species->RemoveHomeworld(homeworld_id);
    }

    bool SpeciesCanColonize(const std::string& species_name) {
        Species* species = SpeciesManager::GetSpeciesManager().GetSpecies(species_name);
        if (!species) {
            ErrorLogger() << "SpeciesCanColonize: couldn't get species " << species_name;
            return false;
        }
        return species->CanColonize();
    }

    list GetAllSpecies() {
        list            species_list;
        for (const auto& entry : GetSpeciesManager()) {
            species_list.append(object(entry.first));
        }
        return species_list;
    }

    list GetPlayableSpecies() {
        list            species_list;
        SpeciesManager& species_manager = GetSpeciesManager();
        for (auto it = species_manager.playable_begin();
             it != species_manager.playable_end(); ++it)
        { species_list.append(object(it->first)); }
        return species_list;
    }

    list GetNativeSpecies() {
        list            species_list;
        SpeciesManager& species_manager = GetSpeciesManager();
        for (auto it = species_manager.native_begin();
             it != species_manager.native_end(); ++it)
        { species_list.append(object(it->first)); }
        return species_list;
    }

    //Checks the condition against many objects at once.
    //Checking many systems is more efficient because for example monster fleet plans
    //typically uses WithinStarLaneJumps to exclude placement near empires.
    list FilterIDsWithCondition(const Condition::ConditionBase* cond, const list &obj_ids) {
        list permitted_ids;

        Condition::ObjectSet objs;
        boost::python::stl_input_iterator<int> end;
        for (boost::python::stl_input_iterator<int> id(obj_ids); id != end; ++id) {
            if (auto obj = GetUniverseObject(*id))
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
        if (cond && cond->SourceInvariant())
            cond->Eval(ScriptingContext(), permitted_objs, objs);
        else
            permitted_objs = std::move(objs);

        for (auto &obj : permitted_objs) {
            permitted_ids.append(obj->ID());
        }

        return permitted_ids;
    }

    // Wrappers for Specials / SpecialManager functions
    double SpecialSpawnRate(const std::string special_name) {
        const Special* special = GetSpecial(special_name);
        if (!special) {
            ErrorLogger() << "SpecialSpawnRate: couldn't get special " << special_name;
            return 0.0;
        }
        return special->SpawnRate();
    }

    int SpecialSpawnLimit(const std::string special_name) {
        const Special* special = GetSpecial(special_name);
        if (!special) {
            ErrorLogger() << "SpecialSpawnLimit: couldn't get special " << special_name;
            return 0;
        }
        return special->SpawnLimit();
    }

    list SpecialLocations(const std::string special_name, const list& object_ids) {
        // get special and check if it exists
        const Special* special = GetSpecial(special_name);
        if (!special) {
            ErrorLogger() << "SpecialLocation: couldn't get special " << special_name;
            return list();
        }

        return FilterIDsWithCondition(special->Location(), object_ids);
    }

    bool SpecialHasLocation(const std::string special_name) {
        // get special and check if it exists
        const Special* special = GetSpecial(special_name);
        if (!special) {
            ErrorLogger() << "SpecialHasLocation: couldn't get special " << special_name;
            return false;
        }
        return special->Location();
    }

    list GetAllSpecials() {
        list py_specials;
        for (const auto& special_name : SpecialNames()) {
            py_specials.append(object(special_name));
        }
        return py_specials;
    }

    // Wrappers for Empire class member functions
    void EmpireSetName(int empire_id, const std::string& name) {
        Empire* empire = GetEmpire(empire_id);
        if (!empire) {
            ErrorLogger() << "EmpireSetName: couldn't get empire with ID " << empire_id;
            return;
        }
        empire->SetName(name);
    }

    bool EmpireSetHomeworld(int empire_id, int planet_id, const std::string& species_name) {
        Empire* empire = GetEmpire(empire_id);
        if (!empire) {
            ErrorLogger() << "EmpireSetHomeworld: couldn't get empire with ID " << empire_id;
            return false;
        }
        return SetEmpireHomeworld(empire, planet_id, species_name);
    }

    void EmpireUnlockItem(int empire_id, UnlockableItemType item_type,
                          const std::string& item_name)
    {
        Empire* empire = GetEmpire(empire_id);
        if (!empire) {
            ErrorLogger() << "EmpireUnlockItem: couldn't get empire with ID " << empire_id;
            return;
        }
        ItemSpec item = ItemSpec(item_type, item_name);
        empire->UnlockItem(item);
    }

    void EmpireAddShipDesign(int empire_id, const std::string& design_name) {
        Universe& universe = GetUniverse();

        Empire* empire = GetEmpire(empire_id);
        if (!empire) {
            ErrorLogger() << "EmpireAddShipDesign: couldn't get empire with ID " << empire_id;
            return;
        }

        // check if a ship design with ID ship_design_id has been added to the universe
        const ShipDesign* ship_design = universe.GetGenericShipDesign(design_name);
        if (!ship_design) {
            ErrorLogger() << "EmpireAddShipDesign: no ship design with name " << design_name << " has been added to the universe";
            return;
        }

        universe.SetEmpireKnowledgeOfShipDesign(ship_design->ID(), empire_id);
        empire->AddShipDesign(ship_design->ID());
    }

    // Wrapper for preunlocked items
    list LoadItemSpecList() {
        list py_items;
        auto& items = GetUniverse().InitiallyUnlockedItems();
        for (const auto& item : items) {
            py_items.append(object(item));
        }
        return py_items;
    }

    // Wrapper for starting buildings
    list LoadStartingBuildings() {
        list py_items;
        auto& buildings = GetUniverse().InitiallyUnlockedBuildings();
        for (auto building : buildings) {
            if (GetBuildingType(building.name))
                py_items.append(object(building));
            else
                ErrorLogger() << "The item " << building.name << " in the starting building list is not a building.";
        }
        return py_items;
    }

    // Wrappers for ship designs and premade ship designs
    bool ShipDesignCreate(const std::string& name, const std::string& description,
                          const std::string& hull, const list& py_parts,
                          const std::string& icon, const std::string& model,
                          bool monster)
    {
        Universe& universe = GetUniverse();
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
        for (int i = 0; i < len(py_parts); i++) {
            parts.push_back(extract<std::string>(py_parts[i]));
        }

        // Create the design and add it to the universe
        ShipDesign* design;
        try {
            design = new ShipDesign(std::invalid_argument(""), name, description,
                                    BEFORE_FIRST_TURN, ALL_EMPIRES,
                                    hull, parts, icon, model, true, monster);
        } catch (const std::invalid_argument&) {
            ErrorLogger() << "CreateShipDesign: invalid ship design";
            return false;
        }

        if (!universe.InsertShipDesign(design)) {
            ErrorLogger() << "CreateShipDesign: couldn't insert ship design into universe";
            delete design;
            return false;
        }

        return true;
    }

    list ShipDesignGetPremadeList() {
        list py_ship_designs;
        for (const auto& design : GetPredefinedShipDesignManager().GetOrderedShipDesigns()) {
            py_ship_designs.append(object(design->Name(false)));
        }
        return list(py_ship_designs);
    }

    list ShipDesignGetMonsterList() {
        list py_monster_designs;
        const auto& manager = GetPredefinedShipDesignManager();
        for (const auto& monster : manager.GetOrderedMonsterDesigns()) {
            py_monster_designs.append(object(monster->Name(false)));
        }
        return list(py_monster_designs);
    }

    // Wrappers for starting fleet plans
    class FleetPlanWrapper {
    public:
        // name ctors
        FleetPlanWrapper(FleetPlan* fleet_plan) :
            m_fleet_plan(std::make_shared<FleetPlan>(*fleet_plan))
        {}

        FleetPlanWrapper(const std::string& fleet_name, const list& py_designs)
        {
            std::vector<std::string> designs;
            for (int i = 0; i < len(py_designs); i++) {
                designs.push_back(extract<std::string>(py_designs[i]));
            }
            m_fleet_plan = std::make_shared<FleetPlan>(fleet_name, designs, false);
        }

        // name accessors
        object Name()
        { return object(m_fleet_plan->Name()); }

        list ShipDesigns() {
            list py_designs;
            for (const auto& design_name : m_fleet_plan->ShipDesigns()) {
                py_designs.append(object(design_name));
            }
            return list(py_designs);
        }

        const FleetPlan& GetFleetPlan()
        { return *m_fleet_plan; }

    private:
        // Use shared_ptr insead of unique_ptr because boost::python requires a deleter
        std::shared_ptr<FleetPlan> m_fleet_plan;
    };

    list LoadFleetPlanList() {
        list py_fleet_plans;
        auto&& fleet_plans = GetUniverse().InitiallyUnlockedFleetPlans();
        for (FleetPlan* fleet_plan : fleet_plans) {
            py_fleet_plans.append(FleetPlanWrapper(fleet_plan));
        }
        return py_fleet_plans;
    }

    // Wrappers for starting monster fleet plans
    class MonsterFleetPlanWrapper {
    public:
        // name ctors
        MonsterFleetPlanWrapper(MonsterFleetPlan* monster_fleet_plan) :
            m_monster_fleet_plan(std::make_shared<MonsterFleetPlan>(*monster_fleet_plan))
        {}

        MonsterFleetPlanWrapper(const std::string& fleet_name, const list& py_designs,
                                double spawn_rate, int spawn_limit)
        {
            std::vector<std::string> designs;
            for (int i = 0; i < len(py_designs); i++) {
                designs.push_back(extract<std::string>(py_designs[i]));
            }
            m_monster_fleet_plan =
                std::make_shared<MonsterFleetPlan>(fleet_name, designs, spawn_rate,
                                                   spawn_limit, nullptr, false);
        }

        // name accessors
        object Name()
        { return object(m_monster_fleet_plan->Name()); }

        list ShipDesigns() {
            list py_designs;
            for (const auto& design_name : m_monster_fleet_plan->ShipDesigns()) {
                py_designs.append(object(design_name));
            }
            return list(py_designs);
        }

        double SpawnRate()
        { return m_monster_fleet_plan->SpawnRate(); }

        int SpawnLimit()
        { return m_monster_fleet_plan->SpawnLimit(); }

        list Locations(list systems) {
            return FilterIDsWithCondition(m_monster_fleet_plan->Location(), systems);
        }

        const MonsterFleetPlan& GetMonsterFleetPlan()
        { return *m_monster_fleet_plan; }

    private:
        // Use shared_ptr insead of unique_ptr because boost::python requires a deleter
        std::shared_ptr<MonsterFleetPlan> m_monster_fleet_plan;
    };

    list LoadMonsterFleetPlanList() {
        list py_monster_fleet_plans;
        auto&& monster_fleet_plans = GetUniverse().MonsterFleetPlans();
        for (auto* fleet_plan : monster_fleet_plans) {
            py_monster_fleet_plans.append(MonsterFleetPlanWrapper(fleet_plan));
        }
        return py_monster_fleet_plans;
    }

    // Wrappers for the various universe object classes member funtions
    // This should provide a more safe and consistent set of server side
    // functions to scripters. All wrapper functions work with object ids, so
    // handling with object references and passing them between the languages is
    // avoided.
    //
    // Wrappers for common UniverseObject class member funtions
    object GetName(int object_id) {
        auto obj = GetUniverseObject(object_id);
        if (!obj) {
            ErrorLogger() << "GetName: Couldn't get object with ID " << object_id;
            return object("");
        }
        return object(obj->Name());
    }

    void SetName(int object_id, const std::string& name) {
        auto obj = GetUniverseObject(object_id);
        if (!obj) {
            ErrorLogger() << "RenameUniverseObject: Couldn't get object with ID " << object_id;
            return;
        }
        obj->Rename(name);
    }

    double GetX(int object_id) {
        auto obj = GetUniverseObject(object_id);
        if (!obj) {
            ErrorLogger() << "GetX: Couldn't get object with ID " << object_id;
            return UniverseObject::INVALID_POSITION;
        }
        return obj->X();
    }

    double GetY(int object_id) {
        auto obj = GetUniverseObject(object_id);
        if (!obj) {
            ErrorLogger() << "GetY: Couldn't get object with ID " << object_id;
            return UniverseObject::INVALID_POSITION;
        }
        return obj->Y();
    }

    tuple GetPos(int object_id) {
        auto obj = GetUniverseObject(object_id);
        if (!obj) {
            ErrorLogger() << "GetPos: Couldn't get object with ID " << object_id;
            return make_tuple(UniverseObject::INVALID_POSITION,
                              UniverseObject::INVALID_POSITION);
        }
        return make_tuple(obj->X(), obj->Y());
    }

    int GetOwner(int object_id) {
        auto obj = GetUniverseObject(object_id);
        if (!obj) {
            ErrorLogger() << "GetOwner: Couldn't get object with ID " << object_id;
            return ALL_EMPIRES;
        }
        return obj->Owner();
    }

    void AddSpecial(int object_id, const std::string special_name) {
        // get the universe object and check if it exists
        auto obj = GetUniverseObject(object_id);
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

        float capacity = special->InitialCapacity(object_id);

        obj->AddSpecial(special_name, capacity);
    }

    void RemoveSpecial(int object_id, const std::string special_name) {
        // get the universe object and check if it exists
        auto obj = GetUniverseObject(object_id);
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

    // Wrappers for Universe class
    double GetUniverseWidth()
    { return GetUniverse().UniverseWidth(); }

    void SetUniverseWidth(double width)
    { GetUniverse().SetUniverseWidth(width); }

    double LinearDistance(int system1_id, int system2_id)
    { return GetPathfinder()->LinearDistance(system1_id, system2_id); }

    int JumpDistanceBetweenSystems(int system1_id, int system2_id)
    { return GetPathfinder()->JumpDistanceBetweenSystems(system1_id, system2_id); }

    list GetAllObjects() {
        list py_all_objects;
        for (int object_id : Objects().FindObjectIDs()) {
            py_all_objects.append(object_id);
        }
        return py_all_objects;
    }

    list GetSystems() {
        list py_systems;
        for (int system_id : Objects().FindObjectIDs<System>()) {
            py_systems.append(system_id);
        }
        return py_systems;
    }

    int CreateSystem(StarType star_type, const std::string& star_name, double x, double y) {
        // Check if star type is set to valid value
        if ((star_type == INVALID_STAR_TYPE) || (star_type == NUM_STAR_TYPES)) {
            ErrorLogger() << "CreateSystem : Can't create a system with a star of type " << star_type;
            return INVALID_OBJECT_ID;
        }

        // Create system and insert it into the object map
        auto system = GetUniverse().InsertNew<System>(star_type, star_name, x, y);
        if (!system) {
            ErrorLogger() << "CreateSystem : Attempt to insert system into the object map failed";
            return INVALID_OBJECT_ID;
        }

        return system->SystemID();
    }

    int CreatePlanet(PlanetSize size, PlanetType planet_type, int system_id, int orbit, const std::string& name) {
        auto system = Objects().Object<System>(system_id);

        // Perform some validity checks
        // Check if system with id system_id exists
        if (!system) {
            ErrorLogger() << "CreatePlanet : Couldn't get system with ID " << system_id;
            return INVALID_OBJECT_ID;
        }

        // Check if orbit number is within allowed range
        if ((orbit < 0) || (orbit >= system->Orbits())) {
            ErrorLogger() << "CreatePlanet : There is no orbit " << orbit << " in system " << system_id;
            return INVALID_OBJECT_ID;
        }

        // Check if desired orbit is still empty
        if (system->OrbitOccupied(orbit)) {
            ErrorLogger() << "CreatePlanet : Orbit " << orbit << " of system " << system_id << " already occupied";
            return INVALID_OBJECT_ID;
        }

        // Check if planet size is set to valid value
        if ((size < SZ_TINY) || (size > SZ_GASGIANT)) {
            ErrorLogger() << "CreatePlanet : Can't create a planet of size " << size;
            return INVALID_OBJECT_ID;
        }

        // Check if planet type is set to valid value
        if ((planet_type < PT_SWAMP) || (planet_type > PT_GASGIANT)) {
            ErrorLogger() << "CreatePlanet : Can't create a planet of type " << planet_type;
            return INVALID_OBJECT_ID;
        }

        // Check if planet type and size match
        // if type is gas giant, size must be too, same goes for asteroids
        if (((planet_type == PT_GASGIANT) && (size != SZ_GASGIANT)) || ((planet_type == PT_ASTEROIDS) && (size != SZ_ASTEROIDS))) {
            ErrorLogger() << "CreatePlanet : Planet of type " << planet_type << " can't have size " << size;
            return INVALID_OBJECT_ID;
        }

        // Create planet and insert it into the object map
        auto planet = GetUniverse().InsertNew<Planet>(planet_type, size);
        if (!planet) {
            ErrorLogger() << "CreateSystem : Attempt to insert planet into the object map failed";
            return INVALID_OBJECT_ID;
        }

        // Add planet to system map
        system->Insert(std::shared_ptr<UniverseObject>(planet), orbit);

        // If a name has been specified, set planet name
        if (!(name.empty()))
            planet->Rename(name);

        return planet->ID();
    }

    int CreateBuilding(const std::string& building_type, int planet_id, int empire_id) {
        auto planet = Objects().Object<Planet>(planet_id);
        if (!planet) {
            ErrorLogger() << "CreateBuilding: couldn't get planet with ID " << planet_id;
            return INVALID_OBJECT_ID;
        }

        auto system = GetSystem(planet->SystemID());
        if (!system) {
            ErrorLogger() << "CreateBuilding: couldn't get system for planet";
            return INVALID_OBJECT_ID;
        }

        const Empire* empire = GetEmpire(empire_id);
        if (!empire) {
            ErrorLogger() << "CreateBuilding: couldn't get empire with ID " << empire_id;
            return INVALID_OBJECT_ID;
        }

        auto building = GetUniverse().InsertNew<Building>(empire_id, building_type, empire_id);
        if (!building) {
            ErrorLogger() << "CreateBuilding: couldn't create building";
            return INVALID_OBJECT_ID;
        }

        system->Insert(building);
        planet->AddBuilding(building->ID());
        building->SetPlanetID(planet_id);
        return building->ID();
    }

    int CreateFleet(const std::string& name, int system_id, int empire_id) {
        // Get system and check if it exists
        auto system = Objects().Object<System>(system_id);
        if (!system) {
            ErrorLogger() << "CreateFleet: couldn't get system with ID " << system_id;
            return INVALID_OBJECT_ID;
        }

        // Create new fleet at the position of the specified system
        auto fleet = GetUniverse().InsertNew<Fleet>(name, system->X(), system->Y(), empire_id);
        if (!fleet) {
            ErrorLogger() << "CreateFleet: couldn't create new fleet";
            return INVALID_OBJECT_ID;
        }

        // Insert fleet into specified system
        system->Insert(fleet);

        // check if we got a fleet name...
        if (name.empty()) {
            // ...no name has been specified, so we have to generate one using the new fleet id
            fleet->Rename(UserString("OBJ_FLEET") + " " + std::to_string(fleet->ID()));
        }

        // return fleet ID
        return fleet->ID();
    }

    int CreateShip(const std::string& name, const std::string& design_name, const std::string& species, int fleet_id) {
        Universe& universe = GetUniverse();

        // check if we got a species name, if yes, check if species exists
        if (!species.empty() && !GetSpecies(species)) {
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
        auto fleet = GetFleet(fleet_id);
        if (!fleet) {
            ErrorLogger() << "CreateShip: couldn't get fleet with ID " << fleet_id;
            return INVALID_OBJECT_ID;
        }

        auto system = GetSystem(fleet->SystemID());
        if (!system) {
            ErrorLogger() << "CreateShip: couldn't get system for fleet";
            return INVALID_OBJECT_ID;
        }

        // get owner empire of specified fleet
        int empire_id = fleet->Owner();
        // if we got the id of an actual empire, get the empire object and check if it exists
        Empire* empire = nullptr;
        if (empire_id != ALL_EMPIRES) {
            empire = GetEmpire(empire_id);
            if (!empire) {
                ErrorLogger() << "CreateShip: couldn't get empire with ID " << empire_id;
                return INVALID_OBJECT_ID;
            }
        }

        // create new ship
        auto ship = universe.InsertNew<Ship>(empire_id, ship_design->ID(), species, empire_id);
        if (!ship) {
            ErrorLogger() << "CreateShip: couldn't create new ship";
            return INVALID_OBJECT_ID;
        }
        system->Insert(ship);

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
        fleet->SetAggressive(fleet->HasArmedShips() || fleet->HasFighterShips());
        ship->SetFleetID(fleet->ID());

        // set the meters of the ship to max values
        ship->ResetTargetMaxUnpairedMeters();
        ship->ResetPairedActiveMeters();
        ship->SetShipMetersToMax();

        ship->BackPropagateMeters();

        //return the new ships id
        return ship->ID();
    }

    int CreateMonsterFleet(int system_id)
    { return CreateFleet(UserString("MONSTERS"), system_id, ALL_EMPIRES); }

    int CreateMonster(const std::string& design_name, int fleet_id)
    { return CreateShip(NewMonsterName(), design_name, "", fleet_id); }

    std::shared_ptr<Field> CreateFieldImpl(const std::string& field_type_name,
                                           double x, double y, double size)
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
        auto field = GetUniverse().InsertNew<Field>(field_type->Name(), x, y, size);
        if (!field) {
            ErrorLogger() << "CreateFieldImpl: couldn't create field";
            return nullptr;
        }

        // get the localized version of the field type name and set that as the fields name
        field->Rename(UserString(field_type->Name()));
        return field;
    }

    int CreateField(const std::string& field_type_name, double x, double y, double size) {
        auto field = CreateFieldImpl(field_type_name, x, y, size);
        if (field)
            return field->ID();
        else
            return INVALID_OBJECT_ID;
    }

    int CreateFieldInSystem(const std::string& field_type_name, double size, int system_id) {
        // check if system exists and get system
        auto system = GetSystem(system_id);
        if (!system) {
            ErrorLogger() << "CreateFieldInSystem: couldn't get system with ID" << system_id;
            return INVALID_OBJECT_ID;
        }
        // create the field with the coordinates of the system
        auto field = CreateFieldImpl(field_type_name, system->X(), system->Y(), size);
        if (!field)
            return INVALID_OBJECT_ID;
        system->Insert(field); // insert the field into the system
        return field->ID();
    }

    // Return a list of system ids of universe objects with @p obj_ids.
    list ObjectsGetSystems(const list& obj_ids) {
        list py_systems;
        boost::python::stl_input_iterator<int> end;
        for (boost::python::stl_input_iterator<int> id(obj_ids);
             id != end; ++id) {
            if (auto obj = GetUniverseObject(*id)) {
                py_systems.append(obj->SystemID());
            } else {
                ErrorLogger() << "Passed an invalid universe object id " << *id;
                py_systems.append(INVALID_OBJECT_ID);
            }
        }
        return py_systems;
    }

    // Return all systems within \p jumps of \p sys_ids
    list SystemsWithinJumps(size_t jumps, const list& sys_ids) {
        list py_systems;
        boost::python::stl_input_iterator<int> end;

        std::vector<int> systems;

        for (boost::python::stl_input_iterator<int> id(sys_ids); id != end; ++id) {
            systems.push_back(*id);
        }

        auto systems_in_vicinity = GetPathfinder()->WithinJumps(jumps, systems);

        for (auto system_id : systems_in_vicinity) {
            py_systems.append(system_id);
        }
        return py_systems;
    }

    // Wrappers for System class member functions
    StarType SystemGetStarType(int system_id) {
        auto system = GetSystem(system_id);
        if (!system) {
            ErrorLogger() << "SystemGetStarType: couldn't get system with ID " << system_id;
            return INVALID_STAR_TYPE;
        }
        return system->GetStarType();
    }

    void SystemSetStarType(int system_id, StarType star_type) {
        // Check if star type is set to valid value
        if ((star_type == INVALID_STAR_TYPE) || (star_type == NUM_STAR_TYPES)) {
            ErrorLogger() << "SystemSetStarType : Can't create a system with a star of type " << star_type;
            return;
        }

        auto system = GetSystem(system_id);
        if (!system) {
            ErrorLogger() << "SystemSetStarType : Couldn't get system with ID " << system_id;
            return;
        }

        system->SetStarType(star_type);
    }

    int SystemGetNumOrbits(int system_id) {
        auto system = GetSystem(system_id);
        if (!system) {
            ErrorLogger() << "SystemGetNumOrbits : Couldn't get system with ID " << system_id;
            return 0;
        }
        return system->Orbits();
    }

    list SystemFreeOrbits(int system_id) {
        list py_orbits;
        auto system = GetSystem(system_id);
        if (!system) {
            ErrorLogger() << "SystemFreeOrbits : Couldn't get system with ID " << system_id;
            return py_orbits;
        }
        for (int orbit_idx : system->FreeOrbits())
        { py_orbits.append(orbit_idx); }
        return py_orbits;
    }

    bool SystemOrbitOccupied(int system_id, int orbit) {
        auto system = GetSystem(system_id);
        if (!system) {
            ErrorLogger() << "SystemOrbitOccupied : Couldn't get system with ID " << system_id;
            return 0;
        }
        return system->OrbitOccupied(orbit);
    }

    int SystemOrbitOfPlanet(int system_id, int planet_id) {
        auto system = GetSystem(system_id);
        if (!system) {
            ErrorLogger() << "SystemOrbitOfPlanet : Couldn't get system with ID " << system_id;
            return 0;
        }
        return system->OrbitOfPlanet(planet_id);
    }

    list SystemGetPlanets(int system_id) {
        list py_planets;
        auto system = GetSystem(system_id);
        if (!system) {
            ErrorLogger() << "SystemGetPlanets : Couldn't get system with ID " << system_id;
            return py_planets;
        }
        for (int planet_id : system->PlanetIDs())
        { py_planets.append(planet_id); }
        return py_planets;
    }

    list SystemGetFleets(int system_id) {
        list py_fleets;
        auto system = GetSystem(system_id);
        if (!system) {
            ErrorLogger() << "SystemGetFleets : Couldn't get system with ID " << system_id;
            return py_fleets;
        }
        for (int fleet_id : system->FleetIDs())
        { py_fleets.append(fleet_id); }
        return py_fleets;
    }

    list SystemGetStarlanes(int system_id) {
        list py_starlanes;
        // get source system
        auto system = GetSystem(system_id);
        if (!system) {
            ErrorLogger() << "SystemGetStarlanes : Couldn't get system with ID " << system_id;
            return py_starlanes;
        }
        // get list of systems the source system has starlanes to
        // we actually get a map of ids and a bool indicating if the entry is a starlane (false) or wormhole (true)
        // iterate over the map we got, only copy starlanes to the python list object we are going to return
        for (const auto& lane : system->StarlanesWormholes()) {
            // if the bool value is false, we have a starlane
            // in this case copy the destination system id to our starlane list
            if (!(lane.second)) {
                py_starlanes.append(lane.first);
            }
        }
        return py_starlanes;
    }

    void SystemAddStarlane(int from_sys_id, int to_sys_id) {
        // get source and destination system, check that both exist
        auto from_sys = GetSystem(from_sys_id);
        if (!from_sys) {
            ErrorLogger() << "SystemAddStarlane : Couldn't find system with ID " << from_sys_id;
            return;
        }
        auto to_sys = GetSystem(to_sys_id);
        if (!to_sys) {
            ErrorLogger() << "SystemAddStarlane : Couldn't find system with ID " << to_sys_id;
            return;
        }
        // add the starlane on both ends
        from_sys->AddStarlane(to_sys_id);
        to_sys->AddStarlane(from_sys_id);
    }

    void SystemRemoveStarlane(int from_sys_id, int to_sys_id) {
        // get source and destination system, check that both exist
        auto from_sys = GetSystem(from_sys_id);
        if (!from_sys) {
            ErrorLogger() << "SystemRemoveStarlane : Couldn't find system with ID " << from_sys_id;
            return;
        }
        auto to_sys = GetSystem(to_sys_id);
        if (!to_sys) {
            ErrorLogger() << "SystemRemoveStarlane : Couldn't find system with ID " << to_sys_id;
            return;
        }
        // remove the starlane from both ends
        from_sys->RemoveStarlane(to_sys_id);
        to_sys->RemoveStarlane(from_sys_id);
    }

    // Wrapper for Planet class member functions
    PlanetType PlanetGetType(int planet_id) {
        auto planet = GetPlanet(planet_id);
        if (!planet) {
            ErrorLogger() << "PlanetGetType: Couldn't get planet with ID " << planet_id;
            return INVALID_PLANET_TYPE;
        }
        return planet->Type();
    }

    void PlanetSetType(int planet_id, PlanetType planet_type) {
        auto planet = GetPlanet(planet_id);
        if (!planet) {
            ErrorLogger() << "PlanetSetType: Couldn't get planet with ID " << planet_id;
            return;
        }

        planet->SetType(planet_type);
        if (planet_type == PT_ASTEROIDS)
            planet->SetSize(SZ_ASTEROIDS);
        else if (planet_type == PT_GASGIANT)
            planet->SetSize(SZ_GASGIANT);
        else if (planet->Size() == SZ_ASTEROIDS)
            planet->SetSize(SZ_TINY);
        else if (planet->Size() == SZ_GASGIANT)
            planet->SetSize(SZ_HUGE);
    }

    PlanetSize PlanetGetSize(int planet_id) {
        auto planet = GetPlanet(planet_id);
        if (!planet) {
            ErrorLogger() << "PlanetGetSize: Couldn't get planet with ID " << planet_id;
            return INVALID_PLANET_SIZE;
        }
        return planet->Size();
    }

    void PlanetSetSize(int planet_id, PlanetSize planet_size) {
        auto planet = GetPlanet(planet_id);
        if (!planet) {
            ErrorLogger() << "PlanetSetSize: Couldn't get planet with ID " << planet_id;
            return;
        }

        planet->SetSize(planet_size);
        if (planet_size == SZ_ASTEROIDS)
            planet->SetType(PT_ASTEROIDS);
        else if (planet_size == SZ_GASGIANT)
            planet->SetType(PT_GASGIANT);
        else if ((planet->Type() == PT_ASTEROIDS) || (planet->Type() == PT_GASGIANT))
            planet->SetType(PT_BARREN);
    }

    object PlanetGetSpecies(int planet_id) {
        auto planet = GetPlanet(planet_id);
        if (!planet) {
            ErrorLogger() << "PlanetGetSpecies: Couldn't get planet with ID " << planet_id;
            return object("");
        }
        return object(planet->SpeciesName());
    }

    void PlanetSetSpecies(int planet_id, const std::string& species_name) {
        auto planet = GetPlanet(planet_id);
        if (!planet) {
            ErrorLogger() << "PlanetSetSpecies: Couldn't get planet with ID " << planet_id;
            return;
        }
        planet->SetSpecies(species_name);
    }

    object PlanetGetFocus(int planet_id) {
        auto planet = GetPlanet(planet_id);
        if (!planet) {
            ErrorLogger() << "PlanetGetFocus: Couldn't get planet with ID " << planet_id;
            return object("");
        }
        return object(planet->Focus());
    }

    void PlanetSetFocus(int planet_id, const std::string& focus) {
        auto planet = GetPlanet(planet_id);
        if (!planet) {
            ErrorLogger() << "PlanetSetSpecies: Couldn't get planet with ID " << planet_id;
            return;
        }
        planet->SetFocus(focus);
    }

    list PlanetAvailableFoci(int planet_id) {
        list py_foci;
        auto planet = GetPlanet(planet_id);
        if (!planet) {
            ErrorLogger() << "PlanetAvailableFoci: Couldn't get planet with ID " << planet_id;
            return py_foci;
        }
        for (const std::string& focus : planet->AvailableFoci()) {
            py_foci.append(object(focus));
        }
        return py_foci;
    }

    bool PlanetMakeOutpost(int planet_id, int empire_id) {
        auto planet = GetPlanet(planet_id);
        if (!planet) {
            ErrorLogger() << "PlanetMakeOutpost: couldn't get planet with ID:" << planet_id;
            return false;
        }

        if (!GetEmpire(empire_id)) {
            ErrorLogger() << "PlanetMakeOutpost: couldn't get empire with ID " << empire_id;
            return false;
        }

        return planet->Colonize(empire_id, "", 0.0);
    }

    bool PlanetMakeColony(int planet_id, int empire_id, const std::string& species, double population) {
        auto planet = GetPlanet(planet_id);
        if (!planet) {
            ErrorLogger() << "PlanetMakeColony: couldn't get planet with ID:" << planet_id;
            return false;
        }

        if (!GetEmpire(empire_id)) {
            ErrorLogger() << "PlanetMakeColony: couldn't get empire with ID " << empire_id;
            return false;
        }

        if (!GetSpecies(species)) {
            ErrorLogger() << "PlanetMakeColony: couldn't get species with name: " << species;
            return false;
        }

        if (population < 0.0)
            population = 0.0;

        return planet->Colonize(empire_id, species, population);
    }

    object PlanetCardinalSuffix(int planet_id) {
        auto planet = GetPlanet(planet_id);
        if (!planet) {
            ErrorLogger() << "PlanetCardinalSuffix: couldn't get planet with ID:" << planet_id;
            return object(UserString("ERROR"));
        }

        return object(planet->CardinalSuffix());
    }
}

namespace FreeOrionPython {
    void WrapServer() {
        class_<PlayerSetupData>("PlayerSetupData")
            .def_readonly("player_name",        &PlayerSetupData::m_player_name)
            .def_readonly("empire_name",        &PlayerSetupData::m_empire_name)
            .def_readonly("empire_color",       &PlayerSetupData::m_empire_color)
            .def_readonly("starting_species",   &PlayerSetupData::m_starting_species_name);

        class_<FleetPlanWrapper>("FleetPlan", init<const std::string&, const list&>())
            .def("name",            &FleetPlanWrapper::Name)
            .def("ship_designs",    &FleetPlanWrapper::ShipDesigns);

        class_<MonsterFleetPlanWrapper>("MonsterFleetPlan", init<const std::string&, const list&, double, int>())
            .def("name",                &MonsterFleetPlanWrapper::Name)
            .def("ship_designs",        &MonsterFleetPlanWrapper::ShipDesigns)
            .def("spawn_rate",          &MonsterFleetPlanWrapper::SpawnRate)
            .def("spawn_limit",         &MonsterFleetPlanWrapper::SpawnLimit)
            .def("locations",           &MonsterFleetPlanWrapper::Locations);

        def("get_universe",                         GetUniverse,                    return_value_policy<reference_existing_object>());
        def("get_all_empires",                      GetAllEmpires);
        def("get_empire",                           GetEmpire,                      return_value_policy<reference_existing_object>());

        def("user_string",                          make_function(&UserString,      return_value_policy<copy_const_reference>()));
        def("roman_number",                         RomanNumber);
        def("get_resource_dir",                     GetResourceDirWrapper);

        def("all_empires",                          AllEmpires);
        def("invalid_object",                       InvalidObjectID);
        def("large_meter_value",                    LargeMeterValue);
        def("invalid_position",                     InvalidPosition);

        def("get_galaxy_setup_data",                GetGalaxySetupData,             return_value_policy<reference_existing_object>());
        def("current_turn",                         CurrentTurn);
        def("generate_sitrep",                      GenerateSitRep);
        def("generate_sitrep",                      GenerateSitRep1);
        def("generate_starlanes",                   GenerateStarlanes);

        def("species_preferred_focus",              SpeciesPreferredFocus);
        def("species_get_planet_environment",       SpeciesGetPlanetEnvironment);
        def("species_add_homeworld",                SpeciesAddHomeworld);
        def("species_remove_homeworld",             SpeciesRemoveHomeworld);
        def("species_can_colonize",                 SpeciesCanColonize);
        def("get_all_species",                      GetAllSpecies);
        def("get_playable_species",                 GetPlayableSpecies);
        def("get_native_species",                   GetNativeSpecies);

        def("special_spawn_rate",                   SpecialSpawnRate);
        def("special_spawn_limit",                  SpecialSpawnLimit);
        def("special_locations",                    SpecialLocations);
        def("special_has_location",                 SpecialHasLocation);
        def("get_all_specials",                     GetAllSpecials);

        def("empire_set_name",                      EmpireSetName);
        def("empire_set_homeworld",                 EmpireSetHomeworld);
        def("empire_unlock_item",                   EmpireUnlockItem);
        def("empire_add_ship_design",               EmpireAddShipDesign);

        def("design_create",                        ShipDesignCreate);
        def("design_get_premade_list",              ShipDesignGetPremadeList);
        def("design_get_monster_list",              ShipDesignGetMonsterList);

        def("load_item_spec_list",                  LoadItemSpecList);
        def("load_starting_buildings",              LoadStartingBuildings);
        def("load_fleet_plan_list",                 LoadFleetPlanList);
        def("load_monster_fleet_plan_list",         LoadMonsterFleetPlanList);

        def("get_name",                             GetName);
        def("set_name",                             SetName);
        def("get_x",                                GetX);
        def("get_y",                                GetY);
        def("get_pos",                              GetPos);
        def("get_owner",                            GetOwner);
        def("add_special",                          AddSpecial);
        def("remove_special",                       RemoveSpecial);

        def("get_universe_width",                   GetUniverseWidth);
        def("set_universe_width",                   SetUniverseWidth);
        def("linear_distance",                      LinearDistance);
        def("jump_distance",                        JumpDistanceBetweenSystems);
        def("get_all_objects",                      GetAllObjects);
        def("get_systems",                          GetSystems);
        def("create_system",                        CreateSystem);
        def("create_planet",                        CreatePlanet);
        def("create_building",                      CreateBuilding);
        def("create_fleet",                         CreateFleet);
        def("create_ship",                          CreateShip);
        def("create_monster_fleet",                 CreateMonsterFleet);
        def("create_monster",                       CreateMonster);
        def("create_field",                         CreateField);
        def("create_field_in_system",               CreateFieldInSystem);

        def("objs_get_systems",                     ObjectsGetSystems);

        def("systems_within_jumps_unordered",       SystemsWithinJumps, "Return all systems within ''jumps'' of the systems with ids ''sys_ids''");

        def("sys_get_star_type",                    SystemGetStarType);
        def("sys_set_star_type",                    SystemSetStarType);
        def("sys_get_num_orbits",                   SystemGetNumOrbits);
        def("sys_free_orbits",                      SystemFreeOrbits);
        def("sys_orbit_occupied",                   SystemOrbitOccupied);
        def("sys_orbit_of_planet",                  SystemOrbitOfPlanet);
        def("sys_get_planets",                      SystemGetPlanets);
        def("sys_get_fleets",                       SystemGetFleets);
        def("sys_get_starlanes",                    SystemGetStarlanes);
        def("sys_add_starlane",                     SystemAddStarlane);
        def("sys_remove_starlane",                  SystemRemoveStarlane);

        def("planet_get_type",                      PlanetGetType);
        def("planet_set_type",                      PlanetSetType);
        def("planet_get_size",                      PlanetGetSize);
        def("planet_set_size",                      PlanetSetSize);
        def("planet_get_species",                   PlanetGetSpecies);
        def("planet_set_species",                   PlanetSetSpecies);
        def("planet_get_focus",                     PlanetGetFocus);
        def("planet_set_focus",                     PlanetSetFocus);
        def("planet_available_foci",                PlanetAvailableFoci);
        def("planet_make_outpost",                  PlanetMakeOutpost);
        def("planet_make_colony",                   PlanetMakeColony);
        def("planet_cardinal_suffix",               PlanetCardinalSuffix);
    }
}
