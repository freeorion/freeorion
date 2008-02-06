#include "PythonAI.h"
#include "../util/AppInterface.h"
#include "../util/Directories.h"

#include "../Empire/Empire.h"

#include "../universe/Universe.h"
#include "../universe/UniverseObject.h"
#include "../universe/Fleet.h"
#include "../universe/Ship.h"
#include "../universe/Building.h"
#include "../universe/ResourceCenter.h"
#include "../universe/PopCenter.h"
#include "../universe/Planet.h"
#include "../universe/System.h"
#include "../universe/Special.h"

#include "../universe/Enums.h"

#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include <boost/python/suite/indexing/map_indexing_suite.hpp>

#include <boost/lexical_cast.hpp>

using boost::python::def;
using boost::python::return_value_policy;
using boost::python::copy_const_reference;
using boost::python::reference_existing_object;
using boost::python::return_by_value;
using boost::python::class_;
using boost::python::bases;
using boost::noncopyable;
using boost::python::no_init;
using boost::python::enum_;
using boost::python::vector_indexing_suite;
using boost::python::map_indexing_suite;

namespace {
    //////////////////////////
    //    STL Containers    //
    //////////////////////////

    /* SetWrapper class encapsulates functions that expose the STL std::set<> class to Python in a limited,
       read-only fashion.  The set can be iterated through in Python, and printed. */
    template <typename ElementType>
    class SetWrapper {
    public:
        typedef typename std::set<ElementType> Set;
        typedef typename Set::const_iterator SetIterator;

        static unsigned int size(const Set& self) {
            return static_cast<unsigned int>(self.size());  // ignore warning http://lists.boost.org/Archives/boost/2007/04/120377.php
        }
        static bool empty(const Set& self) { return self.empty(); }
        static bool contains(const Set& self, const ElementType& item) { return self.find(item) != self.end(); }
        static unsigned int count(const Set& self, const ElementType& item) { return self.find(item) == self.end() ? 0u : 1u; }
        static SetIterator begin(const Set& self) { return self.begin(); }
        static SetIterator end(const Set& self) { return self.end(); }
        static std::string to_string(const Set& self) {
            std::string retval = "set([";
            for (SetIterator it = self.begin(); it != self.end(); ++it) {
                if (it != self.end() && it != self.begin()) {
                    retval += ", ";
                }
                try {
                    std::string s = boost::lexical_cast<std::string>(*it);
                    retval += s;
                } catch (...) {
                    retval += "?";
                }
            }
            retval += "])";
            return retval;
        };

        static void Wrap(const std::string& python_name) {
            class_<Set, boost::noncopyable>(python_name.c_str(), no_init)
                .def("__str__",         &to_string)
                .def("__len__",         &size)
                .def("size",            &size)
                .def("empty",           &empty)
                .def("__contains__",    &contains)
                .def("count",           &count)
                .def("__iter__",        boost::python::iterator<Set>())
                ;
        }
    };
}

////////////////////////
// Python AIInterface //
////////////////////////

// disambiguation of overloaded functions
const std::string&      (*AIIntPlayerNameVoid)(void) =          &AIInterface::PlayerName;
const std::string&      (*AIIntPlayerNameInt)(int) =            &AIInterface::PlayerName;

const Empire*           (*AIIntGetEmpireVoid)(void) =           &AIInterface::GetEmpire;
const Empire*           (*AIIntGetEmpireInt)(int) =             &AIInterface::GetEmpire;

const UniverseObject*   (Universe::*UniverseGetObject)(int) =   &Universe::Object;
const Fleet*            (Universe::*UniverseGetFleet)(int) =    &Universe::Object;
const Ship*             (Universe::*UniverseGetShip)(int) =     &Universe::Object;
const Planet*           (Universe::*UniverseGetPlanet)(int) =   &Universe::Object;
const System*           (Universe::*UniverseGetSystem)(int) =   &Universe::Object;
const Building*         (Universe::*UniverseGetBuilding)(int) = &Universe::Object;

// Expose interface for redirecting standard output and error to FreeOrion logging.  Can be imported
// before loading the main FreeOrion AI interface library.
void LogText(const char* text) {
    AIInterface::LogOutput(std::string(text));
}

void ErrorText(const char* text) {
    AIInterface::ErrorOutput(std::string(text));
}

BOOST_PYTHON_MODULE(freeOrionLogger)
{
    def("log",                    LogText);
    def("error",                  ErrorText);
}

// Expose AIInterface and all associated classes to Python
BOOST_PYTHON_MODULE(freeOrionAIInterface)
{
    ///////////////////
    //  AIInterface  //
    ///////////////////
    def("playerName",               AIIntPlayerNameVoid,        return_value_policy<copy_const_reference>());
    def("playerName",               AIIntPlayerNameInt,         return_value_policy<copy_const_reference>());


    def("playerID",                 AIInterface::PlayerID);
    def("empirePlayerID",           AIInterface::EmpirePlayerID);
    def("allPlayerIDs",             AIInterface::AllPlayerIDs,  return_value_policy<return_by_value>());

    def("playerIsAI",               AIInterface::PlayerIsAI);
    def("playerIsHost",             AIInterface::PlayerIsHost);

    def("empireID",                 AIInterface::EmpireID);
    def("playerEmpireID",           AIInterface::PlayerEmpireID);
    def("allEmpireIDs",             AIInterface::AllEmpireIDs,  return_value_policy<return_by_value>());

    def("getEmpire",                AIIntGetEmpireVoid,         return_value_policy<reference_existing_object>());
    def("getEmpire",                AIIntGetEmpireInt,          return_value_policy<reference_existing_object>());

    def("getUniverse",              AIInterface::GetUniverse,   return_value_policy<reference_existing_object>());

    def("currentTurn",              AIInterface::CurrentTurn);

    def("issueFleetMoveOrder",      AIInterface::IssueFleetMoveOrder);
    def("issueRenameOrder",         AIInterface::IssueRenameOrder);
    def("issueNewFleetOrder",       AIInterface::IssueNewFleetOrder);
    def("issueFleetColonizeOrder",  AIInterface::IssueFleetColonizeOrder);

    def("sendChatMessage",          AIInterface::SendPlayerChatMessage);

    def("doneTurn",                 AIInterface::DoneTurn);


    ///////////////////
    //     Empire    //
    ///////////////////
    class_<Empire, noncopyable>("empire", no_init)
        .add_property("name",           make_function(&Empire::Name,        return_value_policy<copy_const_reference>()))
        .add_property("playerName",     make_function(&Empire::PlayerName,  return_value_policy<copy_const_reference>()))

        .add_property("empireID",       &Empire::EmpireID)
        .add_property("homeworldID",    &Empire::HomeworldID)
        .add_property("capitolID",      &Empire::CapitolID)

        .def("buildingTypeAvailable",   &Empire::BuildingTypeAvailable)
        .def("availableBuildingTypes",  &Empire::AvailableBuildingTypes,    return_value_policy<copy_const_reference>()) 
        .def("techResearched",          &Empire::TechResearched)
        .def("availableTechs",          &Empire::AvailableTechs,            return_value_policy<copy_const_reference>()) 
        .def("getTechStatus",           &Empire::GetTechStatus)
        .def("researchStatus",          &Empire::ResearchStatus)

        .def("hasExploredSystem",       &Empire::HasExploredSystem)
    ;

    ////////////////////
    //    Universe    //
    ////////////////////
    class_<Universe, noncopyable>("universe", no_init)
        .def("getObject",               UniverseGetObject,              return_value_policy<reference_existing_object>())
        .def("getFleet",                UniverseGetFleet,               return_value_policy<reference_existing_object>())
        .def("getShip",                 UniverseGetShip,                return_value_policy<reference_existing_object>())
        .def("getPlanet",               UniverseGetPlanet,              return_value_policy<reference_existing_object>())
        .def("getSystem",               UniverseGetSystem,              return_value_policy<reference_existing_object>())
        .def("getBuilding",             UniverseGetBuilding,            return_value_policy<reference_existing_object>())
        .def("getSpecial",              GetSpecial,                     return_value_policy<reference_existing_object>())

        .add_property("allObjectIDs",  make_function(&Universe::FindObjectIDs<UniverseObject>, return_value_policy<return_by_value>()))

        .def("systemHasStarlane",   &Universe::SystemReachable)
        .def("systemsConnected",    &Universe::SystemsConnected)
    ;

    ////////////////////
    // UniverseObject //
    ////////////////////
    class_<UniverseObject, noncopyable>("universeObject", no_init)
        .add_property("id",                 &UniverseObject::ID)
        .add_property("name",               make_function(&UniverseObject::Name,        return_value_policy<copy_const_reference>()))
        .add_property("x",                  &UniverseObject::X)
        .add_property("y",                  &UniverseObject::Y)
        .add_property("systemID",           &UniverseObject::SystemID)
        .add_property("unowned",            &UniverseObject::Unowned)
        .add_property("owners",             make_function(&UniverseObject::Owners,      return_value_policy<copy_const_reference>()))
        .def("ownedBy",                     &UniverseObject::OwnedBy)
        .def("whollyOwnedBy",               &UniverseObject::WhollyOwnedBy)
        .add_property("creationTurn",       &UniverseObject::CreationTurn)
        .add_property("ageInTurns",         &UniverseObject::AgeInTurns)
        .add_property("specials",           make_function(&UniverseObject::Specials,    return_value_policy<copy_const_reference>()))

        .def_readonly("invalidObjectID",  &UniverseObject::INVALID_OBJECT_ID)
        .def_readonly("invalidObjectAge", &UniverseObject::INVALID_OBJECT_AGE)
    ;

    ///////////////////
    //     Fleet     //
    ///////////////////
    class_<Fleet, bases<UniverseObject>, noncopyable>("fleet", no_init)
        .add_property("finalDestinationID",         &Fleet::FinalDestinationID)
        .add_property("nextSystemID",               &Fleet::NextSystemID)
        .add_property("speed",                      &Fleet::Speed)
        .add_property("canChangeDirectionEnRoute",  &Fleet::CanChangeDirectionEnRoute)
        .add_property("hasArmedShips",              &Fleet::HasArmedShips)
        .add_property("numShips",                   &Fleet::NumShips)
        .def("containsShipID",                      &Fleet::ContainsShip)
        .add_property("shipIDs",                    make_function(&Fleet::ShipIDs,      return_value_policy<copy_const_reference>()))
    ;

    //////////////////
    //     Ship     //
    //////////////////
    class_<Ship, bases<UniverseObject>, noncopyable>("ship", no_init)
        .add_property("fleetID",    &Ship::FleetID)
        .add_property("getFleet",   make_function(&Ship::GetFleet,   return_value_policy<reference_existing_object>()))
        .add_property("isArmed",    &Ship::IsArmed)
        .add_property("speed",      &Ship::Speed)
    ;

    //////////////////
    //   Building   //
    //////////////////
    class_<Building, bases<UniverseObject>, noncopyable>("building", no_init)
        .def("getBuildingType",         &Building::GetBuildingType, return_value_policy<reference_existing_object>())
        .add_property("operating",      &Building::Operating)
        .def("getPlanet",               &Building::GetPlanet,       return_value_policy<reference_existing_object>())
    ;

    //////////////////
    // BuildingType //
    //////////////////
    class_<BuildingType, noncopyable>("buildingType", no_init)
        .add_property("name",               make_function(&BuildingType::Name,          return_value_policy<copy_const_reference>()))
        .add_property("description",        make_function(&BuildingType::Description,   return_value_policy<copy_const_reference>()))
        .add_property("buildCost",          &BuildingType::BuildCost)
        .add_property("buildTime",          &BuildingType::BuildTime)
        .add_property("maintenanceCost",    &BuildingType::MaintenanceCost)
        .def("captureResult",               &BuildingType::GetCaptureResult)
    ;

    ////////////////////
    // ResourceCenter //
    ////////////////////
    class_<ResourceCenter, noncopyable>("resourceCenter", no_init)
        .add_property("primaryFocus",       &ResourceCenter::PrimaryFocus)
        .add_property("secondaryFocus",     &ResourceCenter::SecondaryFocus)
    ;

    ///////////////////
    //   PopCenter   //
    ///////////////////
    class_<PopCenter, noncopyable>("popCenter", no_init)
        .add_property("inhabitants",        &PopCenter::Inhabitants)
        .add_property("availableFood",      &PopCenter::AvailableFood)
    ;

    //////////////////
    //    Planet    //
    //////////////////
    class_<Planet, bases<UniverseObject, PopCenter, ResourceCenter>, noncopyable>("planet", no_init)
        .add_property("size",               &Planet::Size)
        .add_property("type",               &Planet::Type)
        .add_property("buildings",          make_function(&Planet::Buildings,   return_value_policy<copy_const_reference>()))
    ;

    //////////////////
    //    System    //
    //////////////////
    class_<System, bases<UniverseObject>, noncopyable>("system", no_init)
        .add_property("starType",           &System::Star)
        .add_property("numOrbits",          &System::Orbits)
        .add_property("numStarlanes",       &System::Starlanes)
        .add_property("numWormholes",       &System::Wormholes)
        .def("HasStarlaneToSystemID",       &System::HasStarlaneTo)
        .def("HasWormholeToSystemID",       &System::HasWormholeTo)
    ;

    //////////////////
    //     Tech     //
    //////////////////
    class_<Tech, noncopyable>("tech", no_init)
        .add_property("name",               make_function(&Tech::Name,              return_value_policy<copy_const_reference>()))
        .add_property("description",        make_function(&Tech::Description,       return_value_policy<copy_const_reference>()))
        .add_property("shortDescription",   make_function(&Tech::ShortDescription,  return_value_policy<copy_const_reference>()))
        .add_property("type",               &Tech::Type)
        .add_property("category",           make_function(&Tech::Category,          return_value_policy<copy_const_reference>()))
        .add_property("researchCost",       &Tech::ResearchCost)
        .add_property("researchTurns",      &Tech::ResearchTurns)
        .add_property("prerequisites",      make_function(&Tech::Prerequisites,     return_value_policy<copy_const_reference>()))
        .add_property("unlockedTechs",      make_function(&Tech::UnlockedTechs,     return_value_policy<copy_const_reference>()))
    ;

    /////////////////
    //   Special   //
    /////////////////
    class_<Special, noncopyable>("special", no_init)
        .add_property("name",               make_function(&Special::Name,           return_value_policy<copy_const_reference>()))
        .add_property("description",        make_function(&Special::Description,    return_value_policy<copy_const_reference>()))
    ;


    ////////////////////
    //     Enums      //
    ////////////////////
    enum_<StarType>("starType")
        .value("blue",      STAR_BLUE)
        .value("white",     STAR_WHITE)
        .value("yellow",    STAR_YELLOW)
        .value("orange",    STAR_ORANGE)
        .value("red",       STAR_RED)
        .value("neutron",   STAR_NEUTRON)
        .value("blackHole", STAR_BLACK)
    ;
    enum_<PlanetSize>("planetSize")
        .value("tiny",      SZ_TINY)
        .value("small",     SZ_SMALL)
        .value("medium",    SZ_MEDIUM)
        .value("large",     SZ_LARGE)
        .value("huge",      SZ_HUGE)
        .value("asteroids", SZ_ASTEROIDS)
        .value("gasGiant",  SZ_GASGIANT)
    ;
    enum_<PlanetType>("planetType")
        .value("swamp",     PT_SWAMP)
        .value("radiated",  PT_RADIATED)
        .value("toxic",     PT_TOXIC)
        .value("inferno",   PT_INFERNO)
        .value("barren",    PT_BARREN)
        .value("tundra",    PT_TUNDRA)
        .value("desert",    PT_DESERT)
        .value("terran",    PT_TERRAN)
        .value("ocean",     PT_OCEAN)
        .value("asteroids", PT_ASTEROIDS)
        .value("gasGiant",  PT_GASGIANT)
    ;
    enum_<PlanetEnvironment>("planetEnvironment")
        .value("uninhabitable", PE_UNINHABITABLE)
        .value("hostile",       PE_HOSTILE)
        .value("poor",          PE_POOR)
        .value("adequate",      PE_ADEQUATE)
        .value("good",          PE_GOOD)
    ;
    enum_<TechType>("techType")
        .value("theory",        TT_THEORY)
        .value("application",   TT_APPLICATION)
        .value("refinement",    TT_REFINEMENT)
    ;
    enum_<TechStatus>("techStatus")
        .value("unresearchable",    TS_UNRESEARCHABLE)
        .value("researchable",      TS_RESEARCHABLE)
        .value("complete",          TS_COMPLETE)
    ;
    enum_<MeterType>("meterType")
        .value("population",    METER_POPULATION)
        .value("farming",       METER_FARMING)
        .value("industry",      METER_INDUSTRY)
        .value("research",      METER_RESEARCH)
        .value("trade",         METER_TRADE)
        .value("mining",        METER_MINING)
        .value("construction",  METER_CONSTRUCTION)
        .value("health",        METER_HEALTH)
        .value("fuel",          METER_FUEL)
        .value("supply",        METER_SUPPLY)
        .value("stealth",       METER_STEALTH)
        .value("detection",     METER_DETECTION)
        .value("shield",        METER_SHIELD)
        .value("defense",       METER_DEFENSE)
    ;
    enum_<FocusType>("focusType")
        .value("balanced",  FOCUS_BALANCED)
        .value("farming",   FOCUS_FARMING)
        .value("industry",  FOCUS_INDUSTRY)
        .value("mining",    FOCUS_MINING)
        .value("research",  FOCUS_RESEARCH)
        .value("trade",     FOCUS_TRADE)
    ;
    enum_<CaptureResult>("captureResult")
        .value("capture",   CR_CAPTURE)
        .value("destroy",   CR_DESTROY)
        .value("retain",    CR_RETAIN)
        .value("share",     CR_SHARE)
    ;


    ////////////////////
    // STL Containers //
    ////////////////////
    class_<std::vector<int> >("IntVec")
        .def(vector_indexing_suite<std::vector<int> >())
    ;
    class_<std::vector<std::string> >("StringVec")
        .def(vector_indexing_suite<std::vector<std::string> >())
    ;

    SetWrapper<int>::Wrap("IntSet");
    SetWrapper<std::string>::Wrap("StringSet");
}
 
///////////////////////
//     PythonAI      //
///////////////////////
PythonAI::PythonAI()
{
    using boost::python::borrowed;

    Py_Initialize();    // initializes Python interpreter, allowing Python functions to be called from C++

    initfreeOrionLogger();          // allows the "freeOrionLogger" C++ module to be imported within Python code
    initfreeOrionAIInterface();     // allows the "freeOrionAIInterface" C++ module to be imported within Python code


    // get access to Python main namespace, which is needed to call other functions below
    try {
        main_module = PyOBJECT((PyHANDLE(borrowed(PyImport_AddModule("__main__")))));
        dict = PyOBJECT(main_module.attr("__dict__"));
    } catch (PyERROR err) {
        Logger().errorStream() << "error with main module or namespace";
        return;
    }

    PyHANDLE handle;

    try {
        handle = PyHANDLE(PyRun_String("import sys", Py_file_input, dict.ptr(), dict.ptr()));
    } catch (PyERROR err) {
        Logger().errorStream() << "error appending default AI script directory to sys.path";
        return;
    }



    // import FreeOrion logging interface and redirect stdout and stderr to FreeOrion Logger
    try {
        handle = PyHANDLE(PyRun_String((std::string("import freeOrionLogger\n") +
                                        "class debugLogger:\n" +
                                        "  def write(self, stng):\n" +
                                        "    freeOrionLogger.log(stng)\n" +
                                        "class errorLogger:\n" +
                                        "  def write(self, stng):\n" +
                                        "    freeOrionLogger.error(stng)\n" +
                                        "sys.stdout = debugLogger()\n" +
                                        "sys.stderr = errorLogger()\n" +
                                        "print 'Python stdout and stderr redirected'\n").c_str()
                                      , Py_file_input, dict.ptr(), dict.ptr()));
    } catch (PyERROR err) {
        Logger().errorStream() << "error redirecting output to logger";
        return;
    }


    // tell Python the path in which to locate AI script file
    std::string AI_path = (GetGlobalDir() / "default" / "AI").native_directory_string();
    std::string python_path_command = "sys.path.append('" + AI_path + "')";
    try {
        handle = PyHANDLE(PyRun_String(python_path_command.c_str(), Py_file_input, dict.ptr(), dict.ptr()));
    } catch (PyERROR err) {
        Logger().errorStream() << "error appending default AI script directory to sys.path";
        return;
    }



    // load Python script of AI functions
    try {
        handle = PyHANDLE(PyRun_String("import FreeOrionAI", Py_file_input, dict.ptr(), dict.ptr()));
    } catch (PyERROR err) {
        Logger().errorStream() << "error importing FreeOrionAI.py into Python";
        PyErr_Print();
        return;
    }

    // initialize AI within Python
    try {
        handle = PyHANDLE(PyRun_String("FreeOrionAI.initFreeOrionAI()", Py_file_input, dict.ptr(), dict.ptr()));
    } catch(PyERROR err) {
        Logger().errorStream() << "error calling FreeOrionAI.initfreeOrionAI()";
        PyErr_Print();
        return;
    }

    Logger().debugStream() << "Initialized Python AI";
}

PythonAI::~PythonAI()
{
    Logger().debugStream() << "Cleaning up / destructing Python AI";
    Py_Finalize();      // stops Python interpreter and release its resources
}

void PythonAI::GenerateOrders()
{
    try {
        PyHANDLE handle = PyHANDLE(PyRun_String("FreeOrionAI.generateOrders()", Py_file_input, dict.ptr(), dict.ptr()));
    } catch(PyERROR err) {
        Logger().errorStream() << "error calling FreeOrionAI.generateOrders()";
        PyErr_Print();
        AIInterface::DoneTurn();
        return;
    }
}

void PythonAI::HandleChatMessage(int sender_id, const std::string& msg)
{}
