#include "PythonUniverseGenerator.h"

#include "System.h"
#include "Planet.h"

#include "../server/ServerApp.h"
#include "../util/Directories.h"
#include "../util/Logger.h"
#include "../util/Random.h"
#include "../util/i18n.h"
#include "../python/PythonSetWrapper.h"
#include "../python/PythonWrappers.h"

#include <vector>
#include <map>
#include <string>
#include <utility>

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
using boost::python::extract;


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
    int     InvalidObjectID()
    { return INVALID_OBJECT_ID; }

    double  MinSystemSeparation()
    { return MIN_SYSTEM_SEPARATION; }

    double  MinHomeSystemSeparation()
    { return MIN_HOME_SYSTEM_SEPARATION; }

    int     MaxSystemOrbits()
    { return MAX_SYSTEM_ORBITS; }

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
            Logger().errorStream() << "PythonUniverseGenerator::GetName : Couldn't get object with ID " << object_id;
            return object("");
        }
        return object(obj->Name());
    }

    void SetName(int object_id, std::string name) {
        TemporaryPtr<UniverseObject> obj = GetUniverseObject(object_id);
        if (!obj) {
            Logger().errorStream() << "PythonUniverseGenerator::RenameUniverseObject : Couldn't get object with ID " << object_id;
            return;
        }
        obj->Rename(name);
    }

    // Wrappers for Universe class member functions
    double GetUniverseWidth()
    { return GetUniverse().UniverseWidth(); }

    void SetUniverseWidth(double width)
    { GetUniverse().SetUniverseWidth(width); }

    int CreateSystem(StarType star_type, const std::string& star_name, double x, double y) {
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
        Universe& universe = GetUniverse();
        TemporaryPtr<System> system = universe.Objects().Object<System>(system_id);

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
        TemporaryPtr<Planet> planet = universe.CreatePlanet(planet_type, size);
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

    // Wrappers for System class member functions
    StarType GetStarType(int system_id) {
        TemporaryPtr<System> system = GetSystem(system_id);
        if (!system) {
            Logger().errorStream() << "PythonUniverseGenerator::GetStarType : Couldn't get system with ID " << system_id;
            return INVALID_STAR_TYPE;
        }
        return system->GetStarType();
    }

    int GetNumOrbits(int system_id) {
        TemporaryPtr<System> system = GetSystem(system_id);
        if (!system) {
            Logger().errorStream() << "PythonUniverseGenerator::GetNumOrbits : Couldn't get system with ID " << system_id;
            return 0;
        }
        return system->Orbits();
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

    class_<PlayerSetupData>("playerSetupData")
        .def_readonly("playerName",         &PlayerSetupData::m_player_name)
        .def_readonly("empireName",         &PlayerSetupData::m_empire_name)
        .def_readonly("empireColor",        &PlayerSetupData::m_empire_color)
        .def_readonly("startingSpecies",    &PlayerSetupData::m_starting_species_name);

    class_<std::map<int, PlayerSetupData>, noncopyable>("playerSetupDataMap", no_init)
        .def(map_indexing_suite<std::map<int, PlayerSetupData>, true>());

    def("getGalaxySetupData",               GetGalaxySetupData,             return_value_policy<reference_existing_object>());
    def("getPlayerSetupData",               GetPlayerSetupData,             return_value_policy<reference_existing_object>());

    def("userString",                       make_function(&UserString,      return_value_policy<copy_const_reference>()));
    def("romanNumber",                      RomanNumber);

    def("invalidObject",                    InvalidObjectID);
    def("minSystemSeparation",              MinSystemSeparation);
    def("minHomeSystemSeparation",          MinHomeSystemSeparation);
    def("maxSystemOrbits",                  MaxSystemOrbits);

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

    def("getName",                          GetName);
    def("setName",                          SetName);

    def("getUniverseWidth",                 GetUniverseWidth);
    def("setUniverseWidth",                 SetUniverseWidth);
    def("createSystem",                     CreateSystem);
    def("createPlanet",                     CreatePlanet);

    def("getStarType",                      GetStarType);
    def("getNumOrbits",                     GetNumOrbits);

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
