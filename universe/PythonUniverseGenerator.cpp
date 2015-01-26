#include "PythonUniverseGenerator.h"

#include "Condition.h"
#include "Species.h"
#include "Special.h"
#include "System.h"
#include "Planet.h"
#include "Building.h"
#include "ShipDesign.h"
#include "Fleet.h"
#include "Ship.h"
#include "Tech.h"

#include "../server/ServerApp.h"
#include "../util/Directories.h"
#include "../util/Logger.h"
#include "../util/Random.h"
#include "../util/i18n.h"
#include "../util/OptionsDB.h"
#include "../python/PythonSetWrapper.h"
#include "../python/PythonWrappers.h"
#include "../parse/Parse.h"

#include "../Empire/Empire.h"
#include "../Empire/EmpireManager.h"

#include <vector>
#include <map>
#include <string>
#include <utility>

#include <boost/lexical_cast.hpp>
#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include <boost/python/suite/indexing/map_indexing_suite.hpp>
#include <boost/python/list.hpp>
#include <boost/python/tuple.hpp>
#include <boost/python/extract.hpp>
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

using boost::python::vector_indexing_suite;
using boost::python::map_indexing_suite;

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


// Helper stuff (classes, functions etc.) exposed to the
// universe generator Python scripts
namespace {
    // Returns the global GalaxySetupData instance
    GalaxySetupData& GetGalaxySetupDataQ()
    { return ServerApp::GetApp()->GetGalaxySetupData(); }

    // Copy of player setup data last passed to GenerateUniverse
    std::map<int, PlayerSetupData> player_setup_data;

    // Returns the global PlayerSetupData map
    std::map<int, PlayerSetupData>& GetPlayerSetupDataQ()
    { return player_setup_data; }

    // Functions that return various important constants
    int     AllEmpires()
    { return ALL_EMPIRES; }

    int     InvalidObjectID()
    { return INVALID_OBJECT_ID; }

    double  MinSystemSeparation()
    { return MIN_SYSTEM_SEPARATION; }

    float   LargeMeterValue()
    { return Meter::LARGE_VALUE; }

    double  InvalidPosition()
    { return UniverseObject::INVALID_POSITION; }

    // Functions exposed to Python to access the universe tables
    int BaseStarTypeDist(StarType star_type)
    { return UniverseDataTables()["BaseStarTypeDist"][0][star_type]; }

    int UniverseAgeModToStarTypeDist(GalaxySetupOption age, StarType star_type)
    { return UniverseDataTables()["UniverseAgeModToStarTypeDist"][age][star_type]; }

    int DensityModToPlanetSizeDist(GalaxySetupOption density, PlanetSize size)
    { return UniverseDataTables()["DensityModToPlanetSizeDist"][density][size]; }

    int StarTypeModToPlanetSizeDist(StarType star_type, PlanetSize size)
    { return UniverseDataTables()["StarTypeModToPlanetSizeDist"][star_type][size]; }

    int GalaxyShapeModToPlanetSizeDist(Shape shape, PlanetSize size)
    { return UniverseDataTables()["GalaxyShapeModToPlanetSizeDist"][shape][size]; }

    int OrbitModToPlanetSizeDist(int orbit, PlanetSize size)
    { return UniverseDataTables()["OrbitModToPlanetSizeDist"][orbit][size]; }

    int PlanetSizeModToPlanetTypeDist(PlanetSize size, PlanetType planet_type)
    { return UniverseDataTables()["PlanetSizeModToPlanetTypeDist"][size][planet_type]; }

    int OrbitModToPlanetTypeDist(int orbit, PlanetType planet_type)
    { return UniverseDataTables()["OrbitModToPlanetTypeDist"][orbit][planet_type]; }

    int StarTypeModToPlanetTypeDist(StarType star_type, PlanetType planet_type)
    { return UniverseDataTables()["StarTypeModToPlanetTypeDist"][star_type][planet_type]; }

    int MaxStarlaneLength()
    { return UniverseDataTables()["MaxStarlaneLength"][0][0]; }

    int NativeFrequency(GalaxySetupOption freq)
    { return UniverseDataTables()["NativeFrequency"][0][freq]; }

    int MonsterFrequency(GalaxySetupOption freq)
    { return UniverseDataTables()["MonsterFrequency"][0][freq]; }

    int SpecialsFrequency(GalaxySetupOption freq)
    { return UniverseDataTables()["SpecialsFrequency"][0][freq]; }

    // Wrappers for Species / SpeciesManager class (member) functions
    object SpeciesPreferredFocus(const std::string& species_name) {
        const Species* species = GetSpecies(species_name);
        if (!species) {
            Logger().errorStream() << "PythonUniverseGenerator::SpeciesPreferredFocus: couldn't get species " << species_name;
            return object("");
        }
        return object(species->PreferredFocus());
    }

    PlanetEnvironment SpeciesGetPlanetEnvironment(const std::string& species_name, PlanetType planet_type) {
        const Species* species = GetSpecies(species_name);
        if (!species) {
            Logger().errorStream() << "PythonUniverseGenerator::SpeciesGetPlanetEnvironment: couldn't get species " << species_name;
            return INVALID_PLANET_ENVIRONMENT;
        }
        return species->GetPlanetEnvironment(planet_type);
    }

    void SpeciesAddHomeworld(const std::string& species_name, int homeworld_id) {
        Species* species = SpeciesManager::GetSpeciesManager().GetSpecies(species_name);
        if (!species) {
            Logger().errorStream() << "PythonUniverseGenerator::SpeciesAddHomeworld: couldn't get species " << species_name;
            return;
        }
        species->AddHomeworld(homeworld_id);
    }

    void SpeciesRemoveHomeworld(const std::string& species_name, int homeworld_id) {
        Species* species = SpeciesManager::GetSpeciesManager().GetSpecies(species_name);
        if (!species) {
            Logger().errorStream() << "PythonUniverseGenerator::SpeciesAddHomeworld: couldn't get species " << species_name;
            return;
        }
        species->RemoveHomeworld(homeworld_id);
    }

    bool SpeciesCanColonize(const std::string& species_name) {
        Species* species = SpeciesManager::GetSpeciesManager().GetSpecies(species_name);
        if (!species) {
            Logger().errorStream() << "PythonUniverseGenerator::SpeciesCanColonize: couldn't get species " << species_name;
            return false;
        }
        return species->CanColonize();
    }

    list GetAllSpecies() {
        list            species_list;
        SpeciesManager& species_manager = GetSpeciesManager();
        for (SpeciesManager::iterator it = species_manager.begin(); it != species_manager.end(); ++it) {
            species_list.append(object(it->first));
        }
        return species_list;
    }

    list GetPlayableSpecies() {
        list            species_list;
        SpeciesManager& species_manager = GetSpeciesManager();
        for (SpeciesManager::playable_iterator it = species_manager.playable_begin(); it != species_manager.playable_end(); ++it) {
            species_list.append(object(it->first));
        }
        return species_list;
    }

    list GetNativeSpecies() {
        list            species_list;
        SpeciesManager& species_manager = GetSpeciesManager();
        for (SpeciesManager::native_iterator it = species_manager.native_begin(); it != species_manager.native_end(); ++it) {
            species_list.append(object(it->first));
        }
        return species_list;
    }

    // Wrappers for Specials / SpecialManager functions
    double SpecialSpawnRate(const std::string special_name) {
        const Special* special = GetSpecial(special_name);
        if (!special) {
            Logger().errorStream() << "PythonUniverseGenerator::SpecialSpawnRate: couldn't get special " << special_name;
            return 0.0;
        }
        return special->SpawnRate();
    }

    int SpecialSpawnLimit(const std::string special_name) {
        const Special* special = GetSpecial(special_name);
        if (!special) {
            Logger().errorStream() << "PythonUniverseGenerator::SpecialSpawnLimit: couldn't get special " << special_name;
            return 0;
        }
        return special->SpawnLimit();
    }

    bool SpecialLocation(const std::string special_name, int object_id) {
        // get special and check if it exists
        const Special* special = GetSpecial(special_name);
        if (!special) {
            Logger().errorStream() << "PythonUniverseGenerator::SpecialLocation: couldn't get special " << special_name;
            return false;
        }

        // get the universe object to test and check if it exists
        TemporaryPtr<UniverseObject> obj = GetUniverseObject(object_id);
        if (!obj) {
            Logger().errorStream() << "PythonUniverseGenerator::SpecialLocation: Couldn't get object with ID " << object_id;
            return false;
        }

        // get special location condition and evaluate it with the specified universe object
        // if no location condition has been defined, no object matches
        const Condition::ConditionBase* location_test = special->Location();
        return (location_test && location_test->Eval(obj));
    }
    
    bool SpecialHasLocation(const std::string special_name) {
        // get special and check if it exists
        const Special* special = GetSpecial(special_name);
        if (!special) {
            Logger().errorStream() << "PythonUniverseGenerator::SpecialHasLocation: couldn't get special " << special_name;
            return false;
        }
        return special->Location();
    }
    
    list GetAllSpecials() {
        list py_specials;
        const std::vector<std::string> special_names = SpecialNames();
        for (std::vector<std::string>::const_iterator it = special_names.begin(); it != special_names.end(); ++it) {
            py_specials.append(object(*it));
        }
        return py_specials;
    }

    // Wrappers for Empire class member functions
    void EmpireSetName(int empire_id, const std::string& name) {
        Empire* empire = Empires().Lookup(empire_id);
        if (!empire) {
            Logger().errorStream() << "PythonUniverseGenerator::EmpireSetName: couldn't get empire with ID " << empire_id;
            return;
        }
        empire->SetName(name);
    }

    bool EmpireSetHomeworld(int empire_id, int planet_id, const std::string& species_name) {
        Empire* empire = Empires().Lookup(empire_id);
        if (!empire) {
            Logger().errorStream() << "PythonUniverseGenerator::EmpireSetHomeworld: couldn't get empire with ID " << empire_id;
            return false;
        }
        return SetEmpireHomeworld(empire, planet_id, species_name);
    }

    void EmpireUnlockItem(int empire_id, UnlockableItemType item_type, const std::string& item_name) {
        Empire* empire = Empires().Lookup(empire_id);
        if (!empire) {
            Logger().errorStream() << "PythonUniverseGenerator::EmpireUnlockItem: couldn't get empire with ID " << empire_id;
            return;
        }
        ItemSpec item = ItemSpec(item_type, item_name);
        empire->UnlockItem(item);
    }

    void EmpireAddShipDesign(int empire_id, const std::string& design_name) {
        Universe& universe = GetUniverse();

        Empire* empire = Empires().Lookup(empire_id);
        if (!empire) {
            Logger().errorStream() << "PythonUniverseGenerator::EmpireAddShipDesign: couldn't get empire with ID " << empire_id;
            return;
        }

        // check if a ship design with ID ship_design_id has been added to the universe
        const ShipDesign* ship_design = universe.GetGenericShipDesign(design_name);
        if (!ship_design) {
            Logger().errorStream() << "PythonUniverseGenerator::EmpireAddShipDesign: no ship design with name " << design_name << " has been added to the universe";
            return;
        }

        universe.SetEmpireKnowledgeOfShipDesign(ship_design->ID(), empire_id);
        empire->AddShipDesign(ship_design->ID());
    }

    // Wrapper for preunlocked items
    list LoadItemSpecList(const std::string& file_name) {
        list py_items;
        std::vector<ItemSpec> items;
        parse::items(GetResourceDir() / file_name, items);
        for (unsigned int i = 0; i < items.size(); i++) {
            py_items.append(object(items[i]));
        }
        return py_items;
    }

    // Wrappers for ship designs and premade ship designs
    bool ShipDesignCreate(const std::string& name, const std::string& description, const std::string& hull,
                          const list& py_parts,    const std::string& icon,        const std::string& model,
                          bool  monster)
    {
        Universe& universe = GetUniverse();
        // Check for empty name
        if (name.empty()) {
            Logger().errorStream() << "PythonUniverseGenerator::CreateShipDesign: tried to create ship design without a name";
            return false;
        }

        // check if a ship design with the same name has already been added to the universe
        if (universe.GetGenericShipDesign(name)) {
            Logger().errorStream() << "PythonUniverseGenerator::CreateShipDesign: a ship design with the name " << name
                                   << " has already been added to the universe";
            return false;
        }

        // copy parts list from Python list to C++ vector
        std::vector<std::string> parts;
        for (int i = 0; i < len(py_parts); i++) {
            parts.push_back(extract<std::string>(py_parts[i]));
        }
        
        // Check if design is valid
        if (!ShipDesign::ValidDesign(hull, parts)) {
            Logger().errorStream() << "PythonUniverseGenerator::CreateShipDesign: invalid ship design";
            return false;
        }
        
        // Create the design and add it to the universe
        ShipDesign* design = new ShipDesign(name, description, BEFORE_FIRST_TURN, ALL_EMPIRES, hull, parts, icon, model, true, monster);
        if (!design) {
            Logger().errorStream() << "PythonUniverseGenerator::CreateShipDesign: couldn't create ship design";
            return false;
        }
        if (universe.InsertShipDesign(design) == ShipDesign::INVALID_DESIGN_ID) {
            Logger().errorStream() << "PythonUniverseGenerator::CreateShipDesign: couldn't insert ship design into universe";
            delete design;
            return false;
        }
        
        return true;
    }

    list ShipDesignGetPremadeList() {
        list py_ship_designs;
        const PredefinedShipDesignManager& manager = GetPredefinedShipDesignManager();
        for (PredefinedShipDesignManager::iterator it = manager.begin(); it != manager.end(); ++it) {
            py_ship_designs.append(object(it->first));
        }
        return list(py_ship_designs);
    }

    list ShipDesignGetMonsterList() {
        list py_monster_designs;
        const PredefinedShipDesignManager& manager = GetPredefinedShipDesignManager();
        for (PredefinedShipDesignManager::iterator it = manager.begin_monsters(); it != manager.end_monsters(); ++it) {
            py_monster_designs.append(object(it->first));
        }
        return list(py_monster_designs);
    }

    // Wrappers for starting fleet plans
    class FleetPlanWrapper {
    public:
        // name ctors
        FleetPlanWrapper(FleetPlan* fleet_plan)
        { m_fleet_plan = fleet_plan; }

        FleetPlanWrapper(const std::string& fleet_name, const list& py_designs)
        {
            std::vector<std::string> designs;
            for (int i = 0; i < len(py_designs); i++) {
                designs.push_back(extract<std::string>(py_designs[i]));
            }
            m_fleet_plan = new FleetPlan(fleet_name, designs, false);
        }

        // dtor
        virtual ~FleetPlanWrapper()
        { delete m_fleet_plan; }

        // name accessors
        object Name()
        { return object(m_fleet_plan->Name()); }

        list ShipDesigns() {
            list py_designs;
            const std::vector<std::string>& designs = m_fleet_plan->ShipDesigns();
            for (unsigned int i = 0; i < designs.size(); i++) {
                py_designs.append(object(designs[i]));
            }
            return list(py_designs);
        }

        const FleetPlan& GetFleetPlan()
        { return *m_fleet_plan; }

    private:
        FleetPlan* m_fleet_plan;
    };

    list LoadFleetPlanList(const std::string& file_name) {
        list py_fleet_plans;
        std::vector<FleetPlan*> fleet_plans;
        parse::fleet_plans(GetResourceDir() / file_name, fleet_plans);
        for (unsigned int i = 0; i < fleet_plans.size(); i++) {
            py_fleet_plans.append(new FleetPlanWrapper(fleet_plans[i]));
        }
        return list(py_fleet_plans);
    }

    // Wrappers for starting monster fleet plans
    class MonsterFleetPlanWrapper {
    public:
        // name ctors
        MonsterFleetPlanWrapper(MonsterFleetPlan* monster_fleet_plan) {
            m_monster_fleet_plan = monster_fleet_plan;
        }

        MonsterFleetPlanWrapper(const std::string& fleet_name, const list& py_designs,
                                double spawn_rate, int spawn_limit)
        {
            std::vector<std::string> designs;
            for (int i = 0; i < len(py_designs); i++) {
                designs.push_back(extract<std::string>(py_designs[i]));
            }
            m_monster_fleet_plan =
                new MonsterFleetPlan(fleet_name, designs, spawn_rate,
                                     spawn_limit, 0, false);
        }

        // dtor
        virtual ~MonsterFleetPlanWrapper()
        { delete m_monster_fleet_plan; }

        // name accessors
        object Name()
        { return object(m_monster_fleet_plan->Name()); }

        list ShipDesigns() {
            list py_designs;
            const std::vector<std::string>& designs = m_monster_fleet_plan->ShipDesigns();
            for (unsigned int i = 0; i < designs.size(); i++) {
                py_designs.append(object(designs[i]));
            }
            return list(py_designs);
        }

        double SpawnRate()
        { return m_monster_fleet_plan->SpawnRate(); }

        int SpawnLimit()
        { return m_monster_fleet_plan->SpawnLimit(); }
        
        bool Location(int object_id) {
            // get the universe object to test and check if it exists
            TemporaryPtr<UniverseObject> obj = GetUniverseObject(object_id);
            if (!obj) {
                Logger().errorStream() << "PythonUniverseGenerator::MonsterFleetPlanWrapper::Location: Couldn't get object with ID " << object_id;
                return false;
            }

            // get location condition and evaluate it with the specified universe object
            // if no location condition has been defined, any object matches
            const Condition::ConditionBase* location_test = m_monster_fleet_plan->Location();
            return (!location_test || location_test->Eval(obj));
        }
        
        const MonsterFleetPlan& GetMonsterFleetPlan()
        { return *m_monster_fleet_plan; }

    private:
        MonsterFleetPlan* m_monster_fleet_plan;
    };
    
    list LoadMonsterFleetPlanList(const std::string& file_name) {
        list py_monster_fleet_plans;
        std::vector<MonsterFleetPlan*> monster_fleet_plans;
        parse::monster_fleet_plans(GetResourceDir() / file_name, monster_fleet_plans);
        for (unsigned int i = 0; i < monster_fleet_plans.size(); i++) {
            py_monster_fleet_plans.append(new MonsterFleetPlanWrapper(monster_fleet_plans[i]));
        }
        return list(py_monster_fleet_plans);
    }

    // Wrappers for the various universe object classes member funtions
    // This should provide a more safe and consistent set of universe generation
    // functions to scripters. All wrapper functions work with object ids, so
    // handling with object references and passing them between the languages is
    // avoided
    //
    // Wrappers for common UniverseObject class member funtions
    object GetName(int object_id) {
        TemporaryPtr<UniverseObject> obj = GetUniverseObject(object_id);
        if (!obj) {
            Logger().errorStream() << "PythonUniverseGenerator::GetName: Couldn't get object with ID " << object_id;
            return object("");
        }
        return object(obj->Name());
    }

    void SetName(int object_id, const std::string& name) {
        TemporaryPtr<UniverseObject> obj = GetUniverseObject(object_id);
        if (!obj) {
            Logger().errorStream() << "PythonUniverseGenerator::RenameUniverseObject: Couldn't get object with ID " << object_id;
            return;
        }
        obj->Rename(name);
    }

    double GetX(int object_id) {
        TemporaryPtr<UniverseObject> obj = GetUniverseObject(object_id);
        if (!obj) {
            Logger().errorStream() << "PythonUniverseGenerator::GetX: Couldn't get object with ID " << object_id;
            return UniverseObject::INVALID_POSITION;
        }
        return obj->X();
    }

    double GetY(int object_id) {
        TemporaryPtr<UniverseObject> obj = GetUniverseObject(object_id);
        if (!obj) {
            Logger().errorStream() << "PythonUniverseGenerator::GetY: Couldn't get object with ID " << object_id;
            return UniverseObject::INVALID_POSITION;
        }
        return obj->Y();
    }

    tuple GetPos(int object_id) {
        TemporaryPtr<UniverseObject> obj = GetUniverseObject(object_id);
        if (!obj) {
            Logger().errorStream() << "PythonUniverseGenerator::GetPos: Couldn't get object with ID " << object_id;
            return make_tuple(UniverseObject::INVALID_POSITION, UniverseObject::INVALID_POSITION);
        }
        return make_tuple(obj->X(), obj->Y());
    }

    int GetOwner(int object_id) {
        TemporaryPtr<UniverseObject> obj = GetUniverseObject(object_id);
        if (!obj) {
            Logger().errorStream() << "PythonUniverseGenerator::GetOwner: Couldn't get object with ID " << object_id;
            return ALL_EMPIRES;
        }
        return obj->Owner();
    }

    void AddSpecial(int object_id, const std::string special_name) {
        // get the universe object and check if it exists
        TemporaryPtr<UniverseObject> obj = GetUniverseObject(object_id);
        if (!obj) {
            Logger().errorStream() << "PythonUniverseGenerator::AddSpecial: Couldn't get object with ID " << object_id;
            return;
        }
        // check if the special exists
        if (!GetSpecial(special_name)) {
            Logger().errorStream() << "PythonUniverseGenerator::AddSpecial: couldn't get special " << special_name;
            return;
        }
        obj->AddSpecial(special_name);
    }

    void RemoveSpecial(int object_id, const std::string special_name) {
        // get the universe object and check if it exists
        TemporaryPtr<UniverseObject> obj = GetUniverseObject(object_id);
        if (!obj) {
            Logger().errorStream() << "PythonUniverseGenerator::RemoveSpecial: Couldn't get object with ID " << object_id;
            return;
        }
        // check if the special exists
        if (!GetSpecial(special_name)) {
            Logger().errorStream() << "PythonUniverseGenerator::RemoveSpecial: couldn't get special " << special_name;
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
    { return GetUniverse().LinearDistance(system1_id, system2_id); }

    int JumpDistanceBetweenSystems(int system1_id, int system2_id)
    { return GetUniverse().JumpDistanceBetweenSystems(system1_id, system2_id); }

    list GetAllObjects() {
        list py_all_objects;
        std::vector<int> all_objects = Objects().FindObjectIDs();
        for (std::vector<int>::iterator it = all_objects.begin(); it != all_objects.end(); ++it) {
            py_all_objects.append(*it);
        }
        return py_all_objects;
    }

    int CreateSystem(StarType star_type, const std::string& star_name, double x, double y) {
        // Check if star type is set to valid value
        if ((star_type == INVALID_STAR_TYPE) || (star_type == NUM_STAR_TYPES)) {
            Logger().errorStream() << "PythonUniverseGenerator::CreateSystem : Can't create a system with a star of type " << star_type;
            return INVALID_OBJECT_ID;
        }

        // Create system and insert it into the object map
        TemporaryPtr<System> system = GetUniverse().CreateSystem(star_type, star_name, x, y);
        if (!system) {
            Logger().errorStream() << "PythonUniverseGenerator::CreateSystem : Attempt to insert system into the object map failed";
            return INVALID_OBJECT_ID;
        }

        return system->SystemID();
    }

    int CreatePlanet(PlanetSize size, PlanetType planet_type, int system_id, int orbit, const std::string& name) {
        TemporaryPtr<System> system = Objects().Object<System>(system_id);

        // Perform some validity checks
        // Check if system with id system_id exists
        if (!system) {
            Logger().errorStream() << "PythonUniverseGenerator::CreatePlanet : Couldn't get system with ID " << system_id;
            return INVALID_OBJECT_ID;
        }

        // Check if orbit number is within allowed range
        if ((orbit < 0) || (orbit >= system->Orbits())) {
            Logger().errorStream() << "PythonUniverseGenerator::CreatePlanet : There is no orbit " << orbit << " in system " << system_id;
            return INVALID_OBJECT_ID;
        }

        // Check if desired orbit is still empty
        if (system->OrbitOccupied(orbit)) {
            Logger().errorStream() << "PythonUniverseGenerator::CreatePlanet : Orbit " << orbit << " of system " << system_id << " already occupied";
            return INVALID_OBJECT_ID;
        }

        // Check if planet size is set to valid value
        if ((size < SZ_TINY) || (size > SZ_GASGIANT)) {
            Logger().errorStream() << "PythonUniverseGenerator::CreatePlanet : Can't create a planet of size " << size;
            return INVALID_OBJECT_ID;
        }

        // Check if planet type is set to valid value
        if ((planet_type < PT_SWAMP) || (planet_type > PT_GASGIANT)) {
            Logger().errorStream() << "PythonUniverseGenerator::CreatePlanet : Can't create a planet of type " << planet_type;
            return INVALID_OBJECT_ID;
        }

        // Check if planet type and size match
        // if type is gas giant, size must be too, same goes for asteroids
        if (((planet_type == PT_GASGIANT) && (size != SZ_GASGIANT)) || ((planet_type == PT_ASTEROIDS) && (size != SZ_ASTEROIDS))) {
            Logger().errorStream() << "PythonUniverseGenerator::CreatePlanet : Planet of type " << planet_type << " can't have size " << size;
            return INVALID_OBJECT_ID;
        }

        // Create planet and insert it into the object map
        TemporaryPtr<Planet> planet = GetUniverse().CreatePlanet(planet_type, size);
        if (!planet) {
            Logger().errorStream() << "PythonUniverseGenerator::CreateSystem : Attempt to insert planet into the object map failed";
            return INVALID_OBJECT_ID;
        }

        // Add planet to system map
        system->Insert(TemporaryPtr<UniverseObject>(planet), orbit);

        // If a name has been specified, set planet name
        if (!(name.empty()))
            planet->Rename(name);

        return planet->ID();
    }

    int CreateBuilding(const std::string& building_type, int planet_id, int empire_id) {
        TemporaryPtr<Planet> planet = Objects().Object<Planet>(planet_id);
        if (!planet) {
            Logger().errorStream() << "PythonUniverseGenerator::CreateBuilding: couldn't get planet with ID " << planet_id;
            return INVALID_OBJECT_ID;
        }

        TemporaryPtr<System> system = GetSystem(planet->SystemID());
        if (!system) {
            Logger().errorStream() << "PythonUniverseGenerator::CreateBuilding: couldn't get system for planet";
            return INVALID_OBJECT_ID;
        }

        const Empire* empire = Empires().Lookup(empire_id);
        if (!empire) {
            Logger().errorStream() << "PythonUniverseGenerator::CreateBuilding: couldn't get empire with ID " << empire_id;
            return INVALID_OBJECT_ID;
        }

        TemporaryPtr<Building> building = GetUniverse().CreateBuilding(empire_id, building_type, empire_id);
        if (!building) {
            Logger().errorStream() << "PythonUniverseGenerator::CreateBuilding: couldn't create building";
            return INVALID_OBJECT_ID;
        }

        system->Insert(building);
        planet->AddBuilding(building->ID());
        building->SetPlanetID(planet_id);
        return building->ID();
    }

    int CreateFleet(const std::string& name, int system_id, int empire_id) {
        // Get system and check if it exists
        TemporaryPtr<System> system = Objects().Object<System>(system_id);
        if (!system) {
            Logger().errorStream() << "PythonUniverseGenerator::CreateFleet: couldn't get system with ID " << system_id;
            return INVALID_OBJECT_ID;
        }

        // Create new fleet at the position of the specified system
        TemporaryPtr<Fleet> fleet = GetUniverse().CreateFleet(name, system->X(), system->Y(), empire_id);
        if (!fleet) {
            Logger().errorStream() << "PythonUniverseGenerator::CreateFleet: couldn't create new fleet";
            return INVALID_OBJECT_ID;
        }

        // Insert fleet into specified system and return fleet ID
        system->Insert(fleet);
        return fleet->ID();
    }

    int CreateShip(const std::string& name, const std::string& design_name, const std::string& species, int fleet_id) {
        Universe& universe = GetUniverse();

        // check if we got a species name, if yes, check if species exists
        if (!species.empty() && !GetSpecies(species)) {
            Logger().errorStream() << "PythonUniverseGenerator::CreateShip: invalid species specified";
            return INVALID_OBJECT_ID;
        }

        // get ship design and check if it exists
        const ShipDesign* ship_design = universe.GetGenericShipDesign(design_name);
        if (!ship_design) {
            Logger().errorStream() << "PythonUniverseGenerator::CreateShip: couldn't get ship design " << design_name;
            return INVALID_OBJECT_ID;
        }

        // get fleet and check if it exists
        TemporaryPtr<Fleet> fleet = GetFleet(fleet_id);
        if (!fleet) {
            Logger().errorStream() << "PythonUniverseGenerator::CreateShip: couldn't get fleet with ID " << fleet_id;
            return INVALID_OBJECT_ID;
        }

        TemporaryPtr<System> system = GetSystem(fleet->SystemID());
        if (!system) {
            Logger().errorStream() << "PythonUniverseGenerator::CreateShip: couldn't get system for fleet";
            return INVALID_OBJECT_ID;
        }

        // get owner empire of specified fleet
        int empire_id = fleet->Owner();
        // if we got the id of an actual empire, get the empire object and check if it exists
        Empire* empire = 0;
        if (empire_id != ALL_EMPIRES) {
            empire = Empires().Lookup(empire_id);
            if (!empire) {
                Logger().errorStream() << "PythonUniverseGenerator::CreateShip: couldn't get empire with ID " << empire_id;
                return INVALID_OBJECT_ID;
            }
        }

        // create new ship
        TemporaryPtr<Ship> ship = universe.CreateShip(empire_id, ship_design->ID(), species, empire_id);
        if (!ship) {
            Logger().errorStream() << "PythonUniverseGenerator::CreateShip: couldn't create new ship";
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
                ship->Rename(UserString("OBJ_SHIP") + " " + boost::lexical_cast<std::string>(ship->ID()));
            }
        } else {
            // ...yes, name has been specified, so use it
            ship->Rename(name);
        }

        // add ship to fleet, this also moves the ship to the
        // fleets location and inserts it into the system
        fleet->AddShip(ship->ID());
        fleet->SetAggressive(fleet->HasArmedShips());
        ship->SetFleetID(fleet->ID());

        //return the new ships id
        return ship->ID();
    }

    int CreateMonsterFleet(int system_id)
    { return CreateFleet(UserString("MONSTERS"), system_id, ALL_EMPIRES); }

    int CreateMonster(const std::string& design_name, int fleet_id)
    { return CreateShip(NewMonsterName(), design_name, "", fleet_id); }

    // Wrappers for System class member functions
    StarType SystemGetStarType(int system_id) {
        TemporaryPtr<System> system = GetSystem(system_id);
        if (!system) {
            Logger().errorStream() << "PythonUniverseGenerator::SystemGetStarType: couldn't get system with ID " << system_id;
            return INVALID_STAR_TYPE;
        }
        return system->GetStarType();
    }

    void SystemSetStarType(int system_id, StarType star_type) {
        // Check if star type is set to valid value
        if ((star_type == INVALID_STAR_TYPE) || (star_type == NUM_STAR_TYPES)) {
            Logger().errorStream() << "PythonUniverseGenerator::SystemSetStarType : Can't create a system with a star of type " << star_type;
            return;
        }

        TemporaryPtr<System> system = GetSystem(system_id);
        if (!system) {
            Logger().errorStream() << "PythonUniverseGenerator::SystemSetStarType : Couldn't get system with ID " << system_id;
            return;
        }

        system->SetStarType(star_type);
    }

    int SystemGetNumOrbits(int system_id) {
        TemporaryPtr<System> system = GetSystem(system_id);
        if (!system) {
            Logger().errorStream() << "PythonUniverseGenerator::SystemGetNumOrbits : Couldn't get system with ID " << system_id;
            return 0;
        }
        return system->Orbits();
    }

    list SystemGetPlanets(int system_id) {
        list py_planets;
        TemporaryPtr<System> system = GetSystem(system_id);
        if (!system) {
            Logger().errorStream() << "PythonUniverseGenerator::SystemGetPlanets : Couldn't get system with ID " << system_id;
            return py_planets;
        }
        const std::set<int>& planets = system->PlanetIDs();
        for (std::set<int>::const_iterator it = planets.begin();
             it != planets.end(); ++it)
        { py_planets.append(*it); }
        return py_planets;
    }

    list SystemGetStarlanes(int system_id) {
        list py_starlanes;
        // get source system
        TemporaryPtr<System> system = GetSystem(system_id);
        if (!system) {
            Logger().errorStream() << "PythonUniverseGenerator::SystemGetStarlanes : Couldn't get system with ID " << system_id;
            return py_starlanes;
        }
        // get list of systems the source system has starlanes to
        // we actually get a map of ids and a bool indicating if the entry is a starlane (false) or wormhole (true)
        const std::map<int, bool>& starlanes = system->StarlanesWormholes();
        // iterate over the map we got, only copy starlanes to the python list object we are going to return
        for (std::map<int, bool>::const_iterator it = starlanes.begin();
             it != starlanes.end(); ++it) {
            // if the bool value is false, we have a starlane
            // in this case copy the destination system id to our starlane list
            if (!(it->second)) {
                py_starlanes.append(it->first);
            }
        }
        return py_starlanes;
    }

    void SystemAddStarlane(int from_sys_id, int to_sys_id) {
        // get source and destination system, check that both exist
        TemporaryPtr<System> from_sys = GetSystem(from_sys_id);
        if (!from_sys) {
            Logger().errorStream() << "PythonUniverseGenerator::SystemAddStarlane : Couldn't find system with ID " << from_sys_id;
            return;
        }
        TemporaryPtr<System> to_sys = GetSystem(to_sys_id);
        if (!to_sys) {
            Logger().errorStream() << "PythonUniverseGenerator::SystemAddStarlane : Couldn't find system with ID " << to_sys_id;
            return;
        }
        // add the starlane on both ends
        from_sys->AddStarlane(to_sys_id);
        to_sys->AddStarlane(from_sys_id);
    }

    void SystemRemoveStarlane(int from_sys_id, int to_sys_id) {
        // get source and destination system, check that both exist
        TemporaryPtr<System> from_sys = GetSystem(from_sys_id);
        if (!from_sys) {
            Logger().errorStream() << "PythonUniverseGenerator::SystemRemoveStarlane : Couldn't find system with ID " << from_sys_id;
            return;
        }
        TemporaryPtr<System> to_sys = GetSystem(to_sys_id);
        if (!to_sys) {
            Logger().errorStream() << "PythonUniverseGenerator::SystemRemoveStarlane : Couldn't find system with ID " << to_sys_id;
            return;
        }
        // remove the starlane from both ends
        from_sys->RemoveStarlane(to_sys_id);
        to_sys->RemoveStarlane(from_sys_id);
    }

    // Wrapper for Planet class member functions
    PlanetType PlanetGetType(int planet_id) {
        TemporaryPtr<Planet> planet = GetPlanet(planet_id);
        if (!planet) {
            Logger().errorStream() << "PythonUniverseGenerator::PlanetGetType: Couldn't get planet with ID " << planet_id;
            return INVALID_PLANET_TYPE;
        }
        return planet->Type();
    }

    void PlanetSetType(int planet_id, PlanetType planet_type) {
        TemporaryPtr<Planet> planet = GetPlanet(planet_id);
        if (!planet) {
            Logger().errorStream() << "PythonUniverseGenerator::PlanetSetType: Couldn't get planet with ID " << planet_id;
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
        TemporaryPtr<Planet> planet = GetPlanet(planet_id);
        if (!planet) {
            Logger().errorStream() << "PythonUniverseGenerator::PlanetGetSize: Couldn't get planet with ID " << planet_id;
            return INVALID_PLANET_SIZE;
        }
        return planet->Size();
    }

    void PlanetSetSize(int planet_id, PlanetSize planet_size) {
        TemporaryPtr<Planet> planet = GetPlanet(planet_id);
        if (!planet) {
            Logger().errorStream() << "PythonUniverseGenerator::PlanetSetSize: Couldn't get planet with ID " << planet_id;
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
        TemporaryPtr<Planet> planet = GetPlanet(planet_id);
        if (!planet) {
            Logger().errorStream() << "PythonUniverseGenerator::PlanetGetSpecies: Couldn't get planet with ID " << planet_id;
            return object("");
        }
        return object(planet->SpeciesName());
    }

    void PlanetSetSpecies(int planet_id, const std::string& species_name) {
        TemporaryPtr<Planet> planet = GetPlanet(planet_id);
        if (!planet) {
            Logger().errorStream() << "PythonUniverseGenerator::PlanetSetSpecies: Couldn't get planet with ID " << planet_id;
            return;
        }
        planet->SetSpecies(species_name);
    }

    object PlanetGetFocus(int planet_id) {
        TemporaryPtr<Planet> planet = GetPlanet(planet_id);
        if (!planet) {
            Logger().errorStream() << "PythonUniverseGenerator::PlanetGetFocus: Couldn't get planet with ID " << planet_id;
            return object("");
        }
        return object(planet->Focus());
    }

    void PlanetSetFocus(int planet_id, const std::string& focus) {
        TemporaryPtr<Planet> planet = GetPlanet(planet_id);
        if (!planet) {
            Logger().errorStream() << "PythonUniverseGenerator::PlanetSetSpecies: Couldn't get planet with ID " << planet_id;
            return;
        }
        planet->SetFocus(focus);
    }

    list PlanetAvailableFoci(int planet_id) {
        list py_foci;
        TemporaryPtr<Planet> planet = GetPlanet(planet_id);
        if (!planet) {
            Logger().errorStream() << "PythonUniverseGenerator::PlanetAvailableFoci: Couldn't get planet with ID " << planet_id;
            return py_foci;
        }
        std::vector<std::string> foci = planet->AvailableFoci();
        for (unsigned int i = 0; i < foci.size(); i++) {
            py_foci.append(object(foci[i]));
        }
        return py_foci;
    }

    bool PlanetMakeOutpost(int planet_id, int empire_id) {
        TemporaryPtr<Planet> planet = GetPlanet(planet_id);
        if (!planet) {
            Logger().errorStream() << "PythonUniverseGenerator::PlanetMakeOutpost: couldn't get planet with ID:" << planet_id;
            return false;
        }

        if (!Empires().Lookup(empire_id)) {
            Logger().errorStream() << "PythonUniverseGenerator::PlanetMakeOutpost: couldn't get empire with ID " << empire_id;
            return false;
        }

        return planet->Colonize(empire_id, "", 0.0);
    }
    
    bool PlanetMakeColony(int planet_id, int empire_id, const std::string& species, double population) {
        TemporaryPtr<Planet> planet = GetPlanet(planet_id);
        if (!planet) {
            Logger().errorStream() << "PythonUniverseGenerator::PlanetMakeColony: couldn't get planet with ID:" << planet_id;
            return false;
        }

        if (!Empires().Lookup(empire_id)) {
            Logger().errorStream() << "PythonUniverseGenerator::PlanetMakeColony: couldn't get empire with ID " << empire_id;
            return false;
        }
        
        if (!GetSpecies(species)) {
            Logger().errorStream() << "PythonUniverseGenerator::PlanetMakeColony: couldn't get species with name: " << species;
            return false;
        }

        if (population < 0.0)
            population = 0.0;

        return planet->Colonize(empire_id, species, population);
    }
}

// Python module for logging functions
BOOST_PYTHON_MODULE(fo_logger) {
    FreeOrionPython::WrapLogger();
}

// Python module providing the universe generator API
BOOST_PYTHON_MODULE(fo_universe_generator) {

    class_<SystemPosition>("SystemPosition", init<double, double>())
        .def_readwrite("x", &SystemPosition::x)
        .def_readwrite("y", &SystemPosition::y);

    class_<std::vector<SystemPosition> >("SystemPositionVec")
        .def(vector_indexing_suite<std::vector<SystemPosition>, true>());

    class_<PlayerSetupData>("PlayerSetupData")
        .def_readonly("player_name",        &PlayerSetupData::m_player_name)
        .def_readonly("empire_name",        &PlayerSetupData::m_empire_name)
        .def_readonly("empire_color",       &PlayerSetupData::m_empire_color)
        .def_readonly("starting_species",   &PlayerSetupData::m_starting_species_name);

    class_<std::map<int, PlayerSetupData>, noncopyable>("PlayerSetupDataMap", no_init)
        .def(map_indexing_suite<std::map<int, PlayerSetupData>, true>());

    class_<ItemSpec>("ItemSpec", init<UnlockableItemType, const std::string&>())
        .def_readonly("type",   &ItemSpec::type)
        .def_readonly("name",   &ItemSpec::name);

    class_<FleetPlanWrapper>("FleetPlan", init<const std::string&, const list&>())
        .def("name",            &FleetPlanWrapper::Name)
        .def("ship_designs",    &FleetPlanWrapper::ShipDesigns);

    class_<MonsterFleetPlanWrapper>("MonsterFleetPlan", init<const std::string&, const list&, double, int>())
        .def("name",            &MonsterFleetPlanWrapper::Name)
        .def("ship_designs",    &MonsterFleetPlanWrapper::ShipDesigns)
        .def("spawn_rate",      &MonsterFleetPlanWrapper::SpawnRate)
        .def("spawn_limit",     &MonsterFleetPlanWrapper::SpawnLimit)
        .def("location",        &MonsterFleetPlanWrapper::Location);

    def("get_galaxy_setup_data",                GetGalaxySetupDataQ,            return_value_policy<reference_existing_object>());
    def("get_player_setup_data",                GetPlayerSetupDataQ,            return_value_policy<reference_existing_object>());

    def("user_string",                          make_function(&UserString,      return_value_policy<copy_const_reference>()));
    def("roman_number",                         RomanNumber);

    def("all_empires",                          AllEmpires);
    def("invalid_object",                       InvalidObjectID);
    def("min_system_separation",                MinSystemSeparation);
    def("large_meter_value",                    LargeMeterValue);
    def("invalid_position",                     InvalidPosition);

    def("base_star_type_dist",                  BaseStarTypeDist);
    def("universe_age_mod_to_star_type_dist",   UniverseAgeModToStarTypeDist);
    def("density_mod_to_planet_size_dist",      DensityModToPlanetSizeDist);
    def("star_type_mod_to_planet_size_dist",    StarTypeModToPlanetSizeDist);
    def("galaxy_shape_mod_to_planet_size_dist", GalaxyShapeModToPlanetSizeDist);
    def("orbit_mod_to_planet_size_dist",        OrbitModToPlanetSizeDist);
    def("planet_size_mod_to_planet_type_dist",  PlanetSizeModToPlanetTypeDist);
    def("orbit_mod_to_planet_type_dist",        OrbitModToPlanetTypeDist);
    def("star_type_mod_to_planet_type_dist",    StarTypeModToPlanetTypeDist);
    def("max_starlane_length",                  MaxStarlaneLength);
    def("native_frequency",                     NativeFrequency);
    def("monster_frequency",                    MonsterFrequency);
    def("specials_frequency",                   SpecialsFrequency);
    def("calc_typical_universe_width",          CalcTypicalUniverseWidth);

    def("spiral_galaxy_calc_positions",         SpiralGalaxyCalcPositions);
    def("elliptical_galaxy_calc_positions",     EllipticalGalaxyCalcPositions);
    def("cluster_galaxy_calc_positions",        ClusterGalaxyCalcPositions);
    def("ring_galaxy_calc_positions",           RingGalaxyCalcPositions);
    def("irregular_galaxy_positions",           IrregularGalaxyPositions);
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
    def("special_location",                     SpecialLocation);
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
    def("create_system",                        CreateSystem);
    def("create_planet",                        CreatePlanet);
    def("create_building",                      CreateBuilding);
    def("create_fleet",                         CreateFleet);
    def("create_ship",                          CreateShip);
    def("create_monster_fleet",                 CreateMonsterFleet);
    def("create_monster",                       CreateMonster);

    def("sys_get_star_type",                    SystemGetStarType);
    def("sys_set_star_type",                    SystemSetStarType);
    def("sys_get_num_orbits",                   SystemGetNumOrbits);
    def("sys_get_planets",                      SystemGetPlanets);
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

    // Enums
    FreeOrionPython::WrapGameStateEnums();

    // GalaxySetupData
    FreeOrionPython::WrapGalaxySetupData();
}


namespace {
    // Some helper objects needed to initialize and run the
    // Python interface. Don't know if or why they have to
    // be exactly this way, more or less copied them over
    // from the Python AI interface.
#ifdef FREEORION_MACOSX
    static char     s_python_home[MAXPATHLEN];
    static char     s_python_program_name[MAXPATHLEN];
#endif
    static dict     s_python_namespace = dict();
    static object   s_python_module = object();

    // Helper function for executing a Python script
    bool PythonExecScript(const std::string script) {
        try { object ignored = exec(script.c_str(), s_python_namespace, s_python_namespace); }
        catch (error_already_set err) {
            PyErr_Print();
            return false;
        }
        return true;
    }

    // Initializes und runs the Python interpreter
    // Prepares the Python environment
    void PythonInit() {
        Logger().debugStream() << "Initializing universe generator Python interface";

        try {
#ifdef FREEORION_MACOSX
            // There have been recurring issues on OSX to get FO to use the
            // Python framework shipped with the app (instead of falling back
            // on the ones provided by the system). These API calls have been
            // added in an attempt to solve the problems. Not sure if they
            // are really required, but better save than sorry.. ;)
            strcpy(s_python_home, GetPythonHome().string().c_str());
            Py_SetPythonHome(s_python_home);
            Logger().debugStream() << "Python home set to " << Py_GetPythonHome();
            strcpy(s_python_program_name, (GetPythonHome() / "Python").string().c_str());
            Py_SetProgramName(s_python_program_name);
            Logger().debugStream() << "Python program name set to " << Py_GetProgramFullPath();
#endif
            // initializes Python interpreter, allowing Python functions to be called from C++
            Py_Initialize();
            Logger().debugStream() << "Python initialized";
            Logger().debugStream() << "Python version: " << Py_GetVersion();
            Logger().debugStream() << "Python prefix: " << Py_GetPrefix();
            Logger().debugStream() << "Python module search path: " << Py_GetPath();
            Logger().debugStream() << "Initializing C++ interfaces for Python";
            initfo_logger();              // allows the "fo_logger" C++ module to be imported within Python code
            initfo_universe_generator();  // allows the "fo_universe_generator" C++ module to be imported within Python code
        }
        catch (...) {
            Logger().errorStream() << "Unable to initialize Python interpreter";
            return;
        }

        try {
            // get main namespace, needed to run other interpreted code
            object py_main = import("__main__");
            s_python_namespace = extract<dict>(py_main.attr("__dict__"));
        }
        catch (error_already_set err) {
            Logger().errorStream() << "Unable to set up main namespace in Python";
            PyErr_Print();
            return;
        }

        // set up logging by redirecting stdout and stderr to exposed logging functions
        std::string script = "import sys\n"
        "import fo_logger\n"
        "class dbgLogger:\n"
        "  def write(self, msg):\n"
        "    fo_logger.log(msg)\n"
        "class errLogger:\n"
        "  def write(self, msg):\n"
        "    fo_logger.error(msg)\n"
        "sys.stdout = dbgLogger()\n"
        "sys.stderr = errLogger()\n"
        "print ('Python stdout and stderr redirected')";
        if (!PythonExecScript(script)) {
            Logger().errorStream() << "Unable to redirect Python stdout and stderr";
            return;
        }

        // set Python current work directory to directory containing
        // the universe generation Python scripts
        std::string universe_generation_script_dir = GetResourceDir().string() + "/universe_generation";
        script = "import os\n"
        "os.chdir(r'" + universe_generation_script_dir + "')\n"
        "print 'Python current directory set to', os.getcwd()";
        if (!PythonExecScript(script)) {
            Logger().errorStream() << "Unable to set Python current directory";
            return;
        }

        // tell Python the path in which to locate universe generator script file
        std::string command = "sys.path.append(r'" + universe_generation_script_dir + "')";
        if (!PythonExecScript(command)) {
            Logger().errorStream() << "Unable to set universe generator script dir";
            return;
        }

        try {
            // import universe generator script file
            s_python_module = import("universe_generator");
        }
        catch (error_already_set err) {
            Logger().errorStream() << "Unable to import universe generator script";
            PyErr_Print();
            return;
        }

        Logger().debugStream() << "Python interface successfully initialized!";
    }

    void PythonCleanup() {
        // stops Python interpreter and release its resources
        Py_Finalize();
        s_python_namespace = dict();
        s_python_module = object();
        Logger().debugStream() << "Cleaned up universe generator Python interface";
    }

    // Wraps call to the Python universe generator error report function
    std::vector<std::string> PythonErrorReport() {
        std::vector<std::string> err_list;
        object f = s_python_module.attr("error_report");
        if (!f) {
            Logger().errorStream() << "Unable to call Python function error_report ";
            return err_list;
        }

        list py_err_list;
        try { py_err_list = extract<list>(f()); }
        catch (error_already_set err) {
            PyErr_Print();
            return err_list;
        }

        for(int i = 0; i < len(py_err_list); i++) {
            err_list.push_back(extract<std::string>(py_err_list[i]));
        }
        return err_list;
    }

    // Wraps call to the main Python universe generator function
    bool PythonCreateUniverse() {
        bool success;
        object f = s_python_module.attr("create_universe");
        if (!f) {
            Logger().errorStream() << "Unable to call Python function create_universe ";
            return false;
        }
        try { success = f(); }
        catch (error_already_set err) {
            success = false;
            PyErr_Print();
        }
        return success;
    }
}


void GenerateUniverse(const std::map<int, PlayerSetupData>& player_setup_data_) {
    Universe& universe = GetUniverse();

    player_setup_data = player_setup_data_;

    // Initialize RNG with provided seed to get reproducible universes
    int seed = 0;
    try {
        seed = boost::lexical_cast<unsigned int>(GetGalaxySetupData().m_seed);
    } catch (...) {
        try {
            boost::hash<std::string> string_hash;
            std::size_t h = string_hash(GetGalaxySetupData().m_seed);
            seed = static_cast<unsigned int>(h);
        } catch (...) {}
    }
    if (GetGalaxySetupData().m_seed.empty()) {
        //ClockSeed();
        // replicate ClockSeed code here so can log the seed used
        boost::posix_time::ptime ltime = boost::posix_time::microsec_clock::local_time();
        std::string new_seed = boost::posix_time::to_simple_string(ltime);
        boost::hash<std::string> string_hash;
        std::size_t h = string_hash(new_seed);
        Logger().debugStream() << "CreateUniverse:: using clock for Seed:" << new_seed;
        seed = static_cast<unsigned int>(h);
        // store seed in galaxy setup data
        ServerApp::GetApp()->GetGalaxySetupData().m_seed = boost::lexical_cast<std::string>(seed);
    }
    Seed(seed);
    Logger().debugStream() << "GenerateUniverse with seed: " << seed;

    // Setup and run Python interpreter
    PythonInit();

    // Reset the universe object for a new universe
    universe.ResetUniverse();
    // Add predefined ship designs to universe
    GetPredefinedShipDesignManager().AddShipDesignsToUniverse();
    // Initialize empire objects for each player
    InitEmpires(player_setup_data);
    // Call the main Python universe generator function
    if (!PythonCreateUniverse()) {
        ServerApp::GetApp()->Networking().SendMessage(ErrorMessage("SERVER_UNIVERSE_GENERATION_ERRORS", false));
    }

    // Stop and clean up Python interpreter
    PythonCleanup();

    Logger().debugStream() << "Applying first turn effects and updating meters";

    // Apply effects for 1st turn.
    universe.ApplyAllEffectsAndUpdateMeters();
    // Set active meters to targets or maxes after first meter effects application
    SetActiveMetersToTargetMaxCurrentValues(universe.Objects());
    universe.BackPropegateObjectMeters();
    Empires().BackPropegateMeters();

    Logger().debugStream() << "Re-applying first turn meter effects and updating meters";

    // Re-apply meter effects, so that results depending on meter values can be
    // re-checked after initial setting of those meter values
    universe.ApplyMeterEffectsAndUpdateMeters();
    // Re-set active meters to targets after re-application of effects
    SetActiveMetersToTargetMaxCurrentValues(universe.Objects());
    // Set the population of unowned planets to a random fraction of their target values.
    SetNativePopulationValues(universe.Objects());

    universe.BackPropegateObjectMeters();
    Empires().BackPropegateMeters();

    if (GetOptionsDB().Get<bool>("verbose-logging")) {
        Logger().debugStream() << "!!!!!!!!!!!!!!!!!!! After setting active meters to targets";
        Logger().debugStream() << universe.Objects().Dump();
    }

    universe.UpdateEmpireObjectVisibilities();
}
