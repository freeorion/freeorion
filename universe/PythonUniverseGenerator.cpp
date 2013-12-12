#include "PythonUniverseGenerator.h"

#include "../server/ServerApp.h"
#include "../util/Directories.h"
#include "../util/Logger.h"
#include "../util/Random.h"
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
    
    // TEMPORARY HACK!!! Need to provide a global accessible positions
    // vector that can be passed to the still required CreateUniverse
    // function of the Universe class. Will be removed once the Python
    // interface is so far completed that this isn't necessary anymore
    std::vector<SystemPosition> g_system_positions;
    
    // Returns reference to the global system positions vector
    std::vector<SystemPosition>& GetSystemPositions()
    { return g_system_positions; }
    
    // Calculates typical universe width based on number of systems
    // A 150 star universe should be 1000 units across
    // TODO: Move to GenerateUniverse.h
    double CalcTypicalUniverseWidth(int size)
    { return (1000.0 / std::sqrt(150.0)) * std::sqrt(static_cast<double>(size)); }
}


// Python module for logging functions
BOOST_PYTHON_MODULE(foLogger) {
    FreeOrionPython::WrapLogger();
}

// Python module providing the universe generator API
BOOST_PYTHON_MODULE(foUniverseGenerator) {

    class_<Universe, noncopyable>("Universe", no_init)
        .add_property("width", &Universe::UniverseWidth, &Universe::SetUniverseWidth);

    class_<SystemPosition>("SystemPosition", init<double, double>())
        .def_readwrite("x", &SystemPosition::x)
        .def_readwrite("y", &SystemPosition::y);

    class_<std::vector<SystemPosition> >("SystemPositionVec")
        .def(vector_indexing_suite<std::vector<SystemPosition>, true>());

    class_<GalaxySetupData>("galaxySetupData")
        .def_readonly ("seed",              &GalaxySetupData::m_seed)
        .def_readwrite("size",              &GalaxySetupData::m_size)
        .def_readwrite("shape",             &GalaxySetupData::m_shape)
        .def_readonly ("age",               &GalaxySetupData::m_age)
        .def_readonly ("starlaneFrequency", &GalaxySetupData::m_starlane_freq)
        .def_readonly ("planetDensity",     &GalaxySetupData::m_planet_density)
        .def_readonly ("specialsFrequency", &GalaxySetupData::m_specials_freq)
        .def_readonly ("monsterFrequency",  &GalaxySetupData::m_monster_freq)
        .def_readonly ("nativeFrequency",   &GalaxySetupData::m_native_freq)
        .def_readonly ("maxAIAgression",    &GalaxySetupData::m_ai_aggr);

    class_<PlayerSetupData>("playerSetupData")
        .def_readonly("playerName",         &PlayerSetupData::m_player_name)
        .def_readonly("empireName",         &PlayerSetupData::m_empire_name)
        .def_readonly("empireColor",        &PlayerSetupData::m_empire_color)
        .def_readonly("startingSpecies",    &PlayerSetupData::m_starting_species_name);

    class_<std::map<int, PlayerSetupData>, noncopyable>("playerSetupDataMap", no_init)
        .def(map_indexing_suite<std::map<int, PlayerSetupData>, true>());

    def("getUniverse",                      GetUniverse,                    return_value_policy<reference_existing_object>());
    def("getGalaxySetupData",               GetGalaxySetupData,             return_value_policy<reference_existing_object>());
    def("getPlayerSetupData",               GetPlayerSetupData,             return_value_policy<reference_existing_object>());
    def("getSystemPositions",               GetSystemPositions,             return_value_policy<reference_existing_object>());
    def("calcTypicalUniverseWidth",         CalcTypicalUniverseWidth);
    def("spiralGalaxyCalcPositions",        SpiralGalaxyCalcPositions);
    def("ellipticalGalaxyCalcPositions",    EllipticalGalaxyCalcPositions);
    def("clusterGalaxyCalcPositions",       ClusterGalaxyCalcPositions);
    def("ringGalaxyCalcPositions",          RingGalaxyCalcPositions);
    def("irregularGalaxyPositions",         IrregularGalaxyPositions);

    // Enums
    FreeOrionPython::WrapGameStateEnums();
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
    
    // Get a reference to the universe copy of the server
    Universe& universe = GetUniverse();

    // Setup and run Python
    PythonInit();
    
    // Reset the universe object for a new universe
    universe.ResetUniverse();
    // Call the main Python universe generator script
    try {
        PythonCreateUniverse();
    }
    catch (error_already_set err) {
        PyErr_Print();
    }
    // TEMPORARY HACK!!! As the Python universe generator interface is not
    // yet complete, we still have to call Universe::CreateUniverse to do
    // all the stuff that hasn't been implemented in the generator yet
    universe.CreateUniverse(g_galaxy_setup_data->m_size,            g_galaxy_setup_data->m_shape,
                            g_galaxy_setup_data->m_age,             g_galaxy_setup_data->m_starlane_freq,
                            g_galaxy_setup_data->m_planet_density,  g_galaxy_setup_data->m_specials_freq,
                            g_galaxy_setup_data->m_monster_freq,    g_galaxy_setup_data->m_native_freq,
                            g_system_positions,                     player_setup_data);

    // Stop and clean up Python
    PythonCleanup();
}