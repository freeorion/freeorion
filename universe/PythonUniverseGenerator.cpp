#include "PythonUniverseGenerator.h"

#include "Species.h"
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
#include <boost/python/extract.hpp>

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
using boost::python::extract;
using boost::python::len;


// Helper stuff (classes, functions etc.) exposed to the
// universe generator Python scripts
namespace {

    // Global pointer to the GalaxySetupData
    // instance that has been passed in
    GalaxySetupData* g_galaxy_setup_data = 0;

    // Returns the global GalaxySetupData instance
    GalaxySetupData& GetGalaxySetupData()
    { return *g_galaxy_setup_data; }

    // Global reference to the player setup data
    // map that has been passed in
    std::map<int, PlayerSetupData> g_player_setup_data;

    // Returns the global PlayerSetupData map
    std::map<int, PlayerSetupData>& GetPlayerSetupData()
    { return g_player_setup_data; }

    // Functions that return various important constants
    int     AllEmpires()
    { return ALL_EMPIRES; }

    int     InvalidObjectID()
    { return INVALID_OBJECT_ID; }

    double  MinSystemSeparation()
    { return MIN_SYSTEM_SEPARATION; }

    // Universe tables
    const std::vector<int>&                 g_base_star_type_dist                   = UniverseDataTables()["BaseStarTypeDist"][0];
    const std::vector<std::vector<int> >&   g_universe_age_mod_to_star_type_dist    = UniverseDataTables()["UniverseAgeModToStarTypeDist"];
    const std::vector<std::vector<int> >&   g_density_mod_to_planet_size_dist       = UniverseDataTables()["DensityModToPlanetSizeDist"];
    const std::vector<std::vector<int> >&   g_star_type_mod_to_planet_size_dist     = UniverseDataTables()["StarTypeModToPlanetSizeDist"];
    const std::vector<std::vector<int> >&   g_orbit_mod_to_planet_size_dist         = UniverseDataTables()["OrbitModToPlanetSizeDist"];
    const std::vector<std::vector<int> >&   g_planet_size_mod_to_planet_type_dist   = UniverseDataTables()["PlanetSizeModToPlanetTypeDist"];
    const std::vector<std::vector<int> >&   g_orbit_mod_to_planet_type_dist         = UniverseDataTables()["OrbitModToPlanetTypeDist"];
    const std::vector<std::vector<int> >&   g_star_type_mod_to_planet_type_dist     = UniverseDataTables()["StarTypeModToPlanetTypeDist"];

    // Functions exposed to Python to access the universe tables
    int BaseStarTypeDist(StarType star_type)
    { return g_base_star_type_dist[star_type]; }

    int UniverseAgeModToStarTypeDist(GalaxySetupOption age, StarType star_type)
    { return g_universe_age_mod_to_star_type_dist[age][star_type]; }

    int DensityModToPlanetSizeDist(GalaxySetupOption density, PlanetSize size)
    { return g_density_mod_to_planet_size_dist[density][size]; }

    int StarTypeModToPlanetSizeDist(StarType star_type, PlanetSize size)
    { return g_star_type_mod_to_planet_size_dist[star_type][size]; }

    int OrbitModToPlanetSizeDist(int orbit, PlanetSize size)
    { return g_orbit_mod_to_planet_size_dist[orbit][size]; }

    int PlanetSizeModToPlanetTypeDist(PlanetSize size, PlanetType planet_type)
    { return g_planet_size_mod_to_planet_type_dist[size][planet_type]; }

    int OrbitModToPlanetTypeDist(int orbit, PlanetType planet_type)
    { return g_orbit_mod_to_planet_type_dist[orbit][planet_type]; }

    int StarTypeModToPlanetTypeDist(StarType star_type, PlanetType planet_type)
    { return g_star_type_mod_to_planet_type_dist[star_type][planet_type]; }

    // Wrappers for Species / SpeciesManager class (member) functions

    object SpeciesPreferredFocus(const std::string& species_name) {
        const Species* species = GetSpecies(species_name);
        if (!species) {
            Logger().errorStream() << "PythonUniverseGenerator::SpeciesPreferredFocus: couldn't get species " << species_name;
            return object("");
        }
        return object(species->PreferredFocus());
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
        for (int i = 0; i < items.size(); i++) {
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
        ShipDesign* design = new ShipDesign(name, description, BEFORE_FIRST_TURN, hull, parts, icon, model, false, monster);
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
    
    // Wrappers for starting fleet and monster fleet plans
    class FleetPlanWrapper {
    public:
        // name ctors
        FleetPlanWrapper(const FleetPlan& fleet_plan) {
            m_fleet_plan = FleetPlan(fleet_plan.Name(), fleet_plan.ShipDesigns(), false);
        }
        
        FleetPlanWrapper(const std::string& fleet_name, const list& py_designs)
        {
            std::vector<std::string> designs;
            for (int i = 0; i < len(py_designs); i++) {
                designs.push_back(extract<std::string>(py_designs[i]));
            }
            m_fleet_plan = FleetPlan(fleet_name, designs, false);
        }
        
        // name accessors
        object Name()
        { return object(m_fleet_plan.Name()); }
        
        list ShipDesigns() {
            list py_designs;
            const std::vector<std::string>& designs = m_fleet_plan.ShipDesigns();
            for (int i = 0; i < designs.size(); i++) {
                py_designs.append(object(designs[i]));
            }
            return list(py_designs);
        }

        const FleetPlan& GetFleetPlan()
        { return m_fleet_plan; }

    private:
        FleetPlan m_fleet_plan;
    };
    
    list LoadFleetPlanList(const std::string& file_name) {
        list py_fleet_plans;
        std::vector<FleetPlan*> fleet_plans;
        parse::fleet_plans(GetResourceDir() / file_name, fleet_plans);
        for (int i = 0; i < fleet_plans.size(); i++) {
            py_fleet_plans.append(FleetPlanWrapper(*(fleet_plans[i])));
            delete fleet_plans[i];
        }
        return list(py_fleet_plans);
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

    int GetOwner(int object_id) {
        TemporaryPtr<UniverseObject> obj = GetUniverseObject(object_id);
        if (!obj) {
            Logger().errorStream() << "PythonUniverseGenerator::GetOwner: Couldn't get object with ID " << object_id;
            return ALL_EMPIRES;
        }
        return obj->Owner();
    }
    
    // Wrappers for Universe class
    double GetUniverseWidth()
    { return GetUniverse().UniverseWidth(); }

    void SetUniverseWidth(double width)
    { GetUniverse().SetUniverseWidth(width); }

    double LinearDistance(int system1_id, int system2_id)
    { return GetUniverse().LinearDistance(system1_id, system2_id); }

    int JumpDistance(int system1_id, int system2_id)
    { return GetUniverse().JumpDistance(system1_id, system2_id); }
    
    int CreateSystem(StarType star_type, const std::string& star_name, double x, double y) {

        // Check if star type is set to valid value
        if ((star_type == INVALID_STAR_TYPE) || (star_type == NUM_STAR_TYPES)) {
            Logger().errorStream() << "PythonUniverseGenerator::CreateSystem : Can't create a system with a star of type " << star_type;
            return INVALID_OBJECT_ID;
        }

        // Create system and insert it into the object map
        TemporaryPtr<System> system = GetUniverse().CreateSystem(star_type, MAX_SYSTEM_ORBITS, star_name, x, y);
        if (!system) {
            std::string err_msg = "PythonUniverseGenerator::CreateSystem : Attempt to insert system into the object map failed";
            Logger().debugStream() << err_msg;
            throw std::runtime_error(err_msg);
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
            std::string err_msg = "PythonUniverseGenerator::CreateSystem : Attempt to insert planet into the object map failed";
            Logger().debugStream() << err_msg;
            throw std::runtime_error(err_msg);
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
        planet->AddBuilding(building->ID());
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
    
        // check if we got a species name
        if (species.empty()) {
            Logger().errorStream() << "PythonUniverseGenerator::CreateShip: no species specified";
            return INVALID_OBJECT_ID;
        }

        // get ship design and check if it exists
        const ShipDesign* ship_design = universe.GetGenericShipDesign(design_name);
        if (!ship_design) {
            Logger().errorStream() << "PythonUniverseGenerator::CreateShipDesign: couldn't get ship design " << design_name;
            return INVALID_OBJECT_ID;
        }

        // get fleet and check if it exists
        TemporaryPtr<Fleet> fleet = GetFleet(fleet_id);
        if (!fleet) {
            Logger().errorStream() << "PythonUniverseGenerator::CreateShip: couldn't get fleet with ID " << fleet_id;
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
                ship->Rename(UserString("SHIP") + " " + boost::lexical_cast<std::string>(ship->ID()));
            }
        } else {
            // ...yes, name has been specified, so use it
            ship->Rename(name);
        }

        // add ship to fleet, this also moves the ship to the
        // fleets location and inserts it into the system
        fleet->AddShip(ship->ID());

        //return the new ships id
        return ship->ID();
    }

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
        std::vector<int> planets = system->FindObjectIDs<Planet>();
        for (int i = 0; i < planets.size(); i++) {
            py_planets.append(planets[i]);
        }
        return py_planets;
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
        for (int i = 0; i < foci.size(); i++) {
            py_foci.append(object(foci[i]));
        }
        return py_foci;
    }

    // Misc. helper functions/wrappers
    //
    // Wrapper function for i18n::RomanNumber
    object RomanNumberWrapper(unsigned int n)
    { return object(RomanNumber(n)); }
}

// Python module for logging functions
BOOST_PYTHON_MODULE(foLogger) {
    FreeOrionPython::WrapLogger();
}

// Python module providing the universe generator API
BOOST_PYTHON_MODULE(foUniverseGenerator) {

    class_<SystemPosition>("SystemPosition", init<double, double>())
        .def_readwrite("x", &SystemPosition::x)
        .def_readwrite("y", &SystemPosition::y);

    class_<std::vector<SystemPosition> >("SystemPositionVec")
        .def(vector_indexing_suite<std::vector<SystemPosition>, true>());

    class_<PlayerSetupData>("PlayerSetupData")
        .def_readonly("playerName",         &PlayerSetupData::m_player_name)
        .def_readonly("empireName",         &PlayerSetupData::m_empire_name)
        .def_readonly("empireColor",        &PlayerSetupData::m_empire_color)
        .def_readonly("startingSpecies",    &PlayerSetupData::m_starting_species_name);

    class_<std::map<int, PlayerSetupData>, noncopyable>("PlayerSetupDataMap", no_init)
        .def(map_indexing_suite<std::map<int, PlayerSetupData>, true>());

    class_<ItemSpec>("ItemSpec", init<UnlockableItemType, const std::string&>())
        .def_readonly("type",   &ItemSpec::type)
        .def_readonly("name",   &ItemSpec::name);

    class_<FleetPlanWrapper>("FleetPlan", init<const std::string&, const list&>())
        .def("name",        &FleetPlanWrapper::Name)
        .def("shipDesigns", &FleetPlanWrapper::ShipDesigns);
    
    def("getGalaxySetupData",               GetGalaxySetupData,             return_value_policy<reference_existing_object>());
    def("getPlayerSetupData",               GetPlayerSetupData,             return_value_policy<reference_existing_object>());

    def("userString",                       make_function(&UserString,      return_value_policy<copy_const_reference>()));
    def("romanNumber",                      RomanNumber);

    def("allEmpires",                       AllEmpires);
    def("invalidObject",                    InvalidObjectID);
    def("minSystemSeparation",              MinSystemSeparation);

    def("baseStarTypeDist",                 BaseStarTypeDist);
    def("universeAgeModToStarTypeDist",     UniverseAgeModToStarTypeDist);
    def("densityModToPlanetSizeDist",       DensityModToPlanetSizeDist);
    def("starTypeModToPlanetSizeDist",      StarTypeModToPlanetSizeDist);
    def("orbitModToPlanetSizeDist",         OrbitModToPlanetSizeDist);
    def("planetSizeModToPlanetTypeDist",    PlanetSizeModToPlanetTypeDist);
    def("orbitModToPlanetTypeDist",         OrbitModToPlanetTypeDist);
    def("starTypeModToPlanetTypeDist",      StarTypeModToPlanetTypeDist);
    def("calcTypicalUniverseWidth",         CalcTypicalUniverseWidth);

    def("spiralGalaxyCalcPositions",        SpiralGalaxyCalcPositions);
    def("ellipticalGalaxyCalcPositions",    EllipticalGalaxyCalcPositions);
    def("clusterGalaxyCalcPositions",       ClusterGalaxyCalcPositions);
    def("ringGalaxyCalcPositions",          RingGalaxyCalcPositions);
    def("irregularGalaxyPositions",         IrregularGalaxyPositions);
    def("generateStarlanes",                GenerateStarlanes);

    def("speciesPreferredFocus",            SpeciesPreferredFocus);
    def("getAllSpecies",                    GetAllSpecies);
    def("getPlayableSpecies",               GetPlayableSpecies);
    def("getNativeSpecies",                 GetNativeSpecies);

    def("empireSetName",                    EmpireSetName);
    def("empireSetHomeworld",               EmpireSetHomeworld);
    def("empireUnlockItem",                 EmpireUnlockItem);
    def("empireAddShipDesign",              EmpireAddShipDesign);

    def("designCreate",                     ShipDesignCreate);
    def("designGetPremadeList",             ShipDesignGetPremadeList);
    def("designGetMosterList",              ShipDesignGetMonsterList);

    def("loadItemSpecList",                 LoadItemSpecList);
    def("loadFleetPlanList",                LoadFleetPlanList);

    def("getName",                          GetName);
    def("setName",                          SetName);
    def("getOwner",                         GetOwner);

    def("getUniverseWidth",                 GetUniverseWidth);
    def("setUniverseWidth",                 SetUniverseWidth);
    def("linearDistance",                   LinearDistance);
    def("jumpDistance",                     JumpDistance);
    def("createSystem",                     CreateSystem);
    def("createPlanet",                     CreatePlanet);
    def("createBuilding",                   CreateBuilding);
    def("createFleet",                      CreateFleet);
    def("createShip",                       CreateShip);

    def("sysGetStarType",                   SystemGetStarType);
    def("sysSetStarType",                   SystemSetStarType);
    def("sysGetNumOrbits",                  SystemGetNumOrbits);
    def("sysGetPlanets",                    SystemGetPlanets);

    def("planetGetType",                    PlanetGetType);
    def("planetSetType",                    PlanetSetType);
    def("planetGetSize",                    PlanetGetSize);
    def("planetSetSize",                    PlanetSetSize);
    def("planetGetSpecies",                 PlanetGetSpecies);
    def("planetSetSpecies",                 PlanetSetSpecies);
    def("planetGetFocus",                   PlanetGetFocus);
    def("planetSetFocus",                   PlanetSetFocus);
    def("planetAvailableFoci",              PlanetAvailableFoci);

    def("createUniverse",                   CreateUniverse);

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

    // Object storing the main Python createUniverse function callable
    static object   PythonCreateUniverse = object();

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
            initfoLogger();             // allows the "foLogger" C++ module to be imported within Python code
            initfoUniverseGenerator();  // allows the "foUniverseGenerator" C++ module to be imported within Python code
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
        "import foLogger\n"
        "class dbgLogger:\n"
        "  def write(self, msg):\n"
        "    foLogger.log(msg)\n"
        "class errLogger:\n"
        "  def write(self, msg):\n"
        "    foLogger.error(msg)\n"
        "sys.stdout = dbgLogger()\n"
        "sys.stderr = errLogger()\n"
        "print ('Python stdout and stderr redirected')";
        if (!PythonExecScript(script)) {
            Logger().errorStream() << "Unable to redirect Python stdout and stderr";
            return;
        }

        // set Python current work directory to resource dir
        script = "import os\n"
        "os.chdir(r'" + (GetResourceDir()).string() + "')\n"
        "print 'Python current directory set to', os.getcwd()";
        if (!PythonExecScript(script)) {
            Logger().errorStream() << "Unable to set Python current directory";
            return;
        }

        // tell Python the path in which to locate universe generator script file
        std::string command = "sys.path.append(r'" + (GetResourceDir()).string() + "')";
        if (!PythonExecScript(command)) {
            Logger().errorStream() << "Unable to set universe generator script dir";
            return;
        }

        try {
            // import universe generator script file
            s_python_module = import("UniverseGenerator");
            PythonCreateUniverse = s_python_module.attr("createUniverse");
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
        PythonCreateUniverse = object();
        Logger().debugStream() << "Cleaned up universe generator Python interface";
    }
}


void GenerateUniverse(GalaxySetupData&                      galaxy_setup_data,
                      const std::map<int, PlayerSetupData>& player_setup_data)
{
    // Set the global GalaxySetupData reference and PlayerSetupData map
    // to the instances we received
    g_galaxy_setup_data = &galaxy_setup_data;
    g_player_setup_data = player_setup_data;

    // Initialize RNG with provided seed to get reproducible universes
    int seed = 0;
    try {
        seed = boost::lexical_cast<unsigned int>(g_galaxy_setup_data->m_seed);
    } catch (...) {
        try {
            boost::hash<std::string> string_hash;
            std::size_t h = string_hash(g_galaxy_setup_data->m_seed);
            seed = static_cast<unsigned int>(h);
        } catch (...) {
        }
    }
    Seed(seed);
    Logger().debugStream() << "GenerateUniverse with seed: " << seed;

    // Setup and run Python interpreter
    PythonInit();

    // Reset the universe object for a new universe
    GetUniverse().ResetUniverse();
    // Add predefined ship designs to universe
    GetPredefinedShipDesignManager().AddShipDesignsToUniverse();
    // Initialize empire objects for each player
    InitEmpires(player_setup_data);
    // Call the main Python universe generator script
    try {
        PythonCreateUniverse();
    }
    catch (error_already_set err) {
        PyErr_Print();
    }

    // Stop and clean up Python interpreter
    PythonCleanup();
}
