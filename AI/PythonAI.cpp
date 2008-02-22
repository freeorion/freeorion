#include "PythonAI.h"
#include "../util/AppInterface.h"
#include "../util/Directories.h"

#include "../Empire/Empire.h"

#include "../universe/Universe.h"
#include "../universe/UniverseObject.h"
#include "../universe/Fleet.h"
#include "../universe/Ship.h"
#include "../universe/ShipDesign.h"
#include "../universe/Building.h"
#include "../universe/ResourceCenter.h"
#include "../universe/PopCenter.h"
#include "../universe/Planet.h"
#include "../universe/System.h"
#include "../universe/Special.h"

#include "../universe/Enums.h"

#include "../python/PythonSetWrapper.h"

#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include <boost/python/suite/indexing/map_indexing_suite.hpp>

#include <boost/function.hpp>
#include <boost/mpl/vector.hpp>

#include <boost/lexical_cast.hpp>

using boost::python::class_;
using boost::python::bases;
using boost::python::def;
using boost::python::iterator;
using boost::python::no_init;
using boost::noncopyable;
using boost::python::return_value_policy;
using boost::python::copy_const_reference;
using boost::python::reference_existing_object;
using boost::python::return_by_value;
using boost::python::return_internal_reference;
using boost::python::enum_;
using boost::python::vector_indexing_suite;
using boost::python::map_indexing_suite;
using boost::python::object;
using boost::python::import;
using boost::python::error_already_set;
using boost::python::exec;
using boost::python::dict;
using boost::python::extract;

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

bool                    (Empire::*BuildableItemBuildingOrbital)(BuildType, const std::string&, int) const =
                                                                &Empire::BuildableItem;
bool                    (Empire::*BuildableItemShip)(BuildType, int, int) const =
                                                                &Empire::BuildableItem;

const ProductionQueue::Element&
                        (ProductionQueue::*ProductionQueueOperatorSquareBrackets)(int) const =
                                                                &ProductionQueue::operator[];


int                     (*AIIntNewFleet)(const std::string&, int) =
                                                                &AIInterface::IssueNewFleetOrder;

// helper functions for exposing queues.

// Research queue tests whether it contains a Tech but Python needs a __contains__ function that takes a 
// *Queue::Element.  This helper functions take an Element and returns the associated Tech.
const Tech*             TechFromResearchQueueElement(const ResearchQueue::Element& element) { return element.tech; }

// Concatenate functions to create one that takes two parameters.  The first parameter is a ResearchQueue*, which
// is passed directly to ResearchQueue::InQueue as the this pointer.  The second parameter is a
// ResearchQueue::Element which is passed into TechFromResearchQueueElement, which reeturns a Tech*, which is
// passed into ResearchQueue::InQueue as the second parameter.
boost::function<bool(const ResearchQueue*, const ResearchQueue::Element&)> InQueueFromResearchQueueElementFunc =
    boost::bind(&ResearchQueue::InQueue, _1, boost::bind(TechFromResearchQueueElement, _2));

// ProductionQueue::Element contains a ProductionItem which contains details of the item on the queue.  Need helper
// functions to get the details about the item in the Element without adding extra pointless exposed classes to
// the Python interface
BuildType               BuildTypeFromProductionQueueElement(const ProductionQueue::Element& element) { return element.item.build_type; }
const std::string&      NameFromProductionQueueElement(const ProductionQueue::Element& element) { return element.item.name; }
int                     DesignIDFromProductionQueueElement(const ProductionQueue::Element& element) { return element.item.design_id; }


namespace {
    // static s_save_state_string, getter and setter to be exposed to Python
    static std::string s_save_state_string("");
    static const std::string& GetStaticSaveStateString() {
        //Logger().debugStream() << "Python-exposed GetSaveStateString() returning " << s_save_state_string;
        return s_save_state_string;
    }
    static void SetStaticSaveStateString(const std::string& new_state_string) {
        s_save_state_string = new_state_string;
        //Logger().debugStream() << "Python-exposed SetSaveStateString(" << s_save_state_string << ")";
    }
}

// Expose interface for redirecting standard output and error to FreeOrion logging.  Can be imported
// before loading the main FreeOrion AI interface library.
static const int MAX_SINGLE_CHUNK_TEXT_SIZE = 1000; 
static std::string log_buffer("");
void LogText(const char* text) {
    // Python sends text as several null-terminated array of char which need to be
    // concatenated before they are output to the logger.  There's probably a better
    // way to do this, but I don't know what it is, and this seems reasonably safe...
    if (!text) return;
    for (int i = 0; i < MAX_SINGLE_CHUNK_TEXT_SIZE; ++i) {
        if (text[i] == '\0') break;
        if (text[i] == '\n' || i == MAX_SINGLE_CHUNK_TEXT_SIZE - 1) {
            AIInterface::LogOutput(log_buffer);
            log_buffer = "";
        } else {
            log_buffer += text[i];
        }
    }
}

static std::string error_buffer("");
void ErrorText(const char* text) {
    // Python sends text as several null-terminated array of char which need to be
    // concatenated before they are output to the logger.  There's probably a better
    // way to do this, but I don't know what it is, and this seems reasonably safe...
   if (!text) return;
    for (int i = 0; i < MAX_SINGLE_CHUNK_TEXT_SIZE; ++i) {
        if (text[i] == '\0') break;
        if (text[i] == '\n' || i == MAX_SINGLE_CHUNK_TEXT_SIZE - 1) {
            AIInterface::ErrorOutput(error_buffer);
            error_buffer = "";
        } else {
            error_buffer += text[i];
        }
    }
}

// Expose minimal debug and error (stdout and stderr respectively) sinks so Python text output can be
// recovered and saved in c++
BOOST_PYTHON_MODULE(freeOrionLogger)
{
    def("log",                    LogText);
    def("error",                  ErrorText);
}

/** Expose AIInterface and all associated classes to Python.
 *
 * CallPolicies:
 *
 * return_value_policy<copy_const_reference>        when returning a relatively small object, such as a string,
 *                                                  that is returned by const reference or pointer
 *
 * return_value_policy<return_by_value>             when returning either a simple data type or a temporary object
 *                                                  in a function that will go out of scope after being returned
 *
 * return_internal_reference<>                      when returning an object or data that is a member of the object
 *                                                  on which the function is called (and shares its lifetime)
 *
 * return_value_policy<reference_existing_object>   when returning an object from a non-member function, or a 
 *                                                  member function where the returned object's lifetime is not
 *                                                  fixed to the lifetime of the object on which the function is
 *                                                  called
 */
BOOST_PYTHON_MODULE(freeOrionAIInterface)
{
    ///////////////////
    //  AIInterface  //
    ///////////////////
    def("playerName",               AIIntPlayerNameVoid,            return_value_policy<copy_const_reference>());
    def("playerName",               AIIntPlayerNameInt,             return_value_policy<copy_const_reference>());

    def("playerID",                 AIInterface::PlayerID);
    def("empirePlayerID",           AIInterface::EmpirePlayerID);
    def("allPlayerIDs",             AIInterface::AllPlayerIDs,      return_value_policy<return_by_value>());

    def("playerIsAI",               AIInterface::PlayerIsAI);
    def("playerIsHost",             AIInterface::PlayerIsHost);

    def("empireID",                 AIInterface::EmpireID);
    def("playerEmpireID",           AIInterface::PlayerEmpireID);
    def("allEmpireIDs",             AIInterface::AllEmpireIDs,      return_value_policy<return_by_value>());

    def("getEmpire",                AIIntGetEmpireVoid,             return_value_policy<reference_existing_object>());
    def("getEmpire",                AIIntGetEmpireInt,              return_value_policy<reference_existing_object>());

    def("getUniverse",              AIInterface::GetUniverse,       return_value_policy<reference_existing_object>());

    def("currentTurn",              AIInterface::CurrentTurn);

    def("issueFleetMoveOrder",      AIInterface::IssueFleetMoveOrder);
    def("issueRenameOrder",         AIInterface::IssueRenameOrder);
    def("issueNewFleetOrder",       AIIntNewFleet);
    def("issueFleetTransferOrder",  AIInterface::IssueFleetTransferOrder);
    def("issueColonizeOrder",       AIInterface::IssueFleetColonizeOrder);
    def("issueChangeFocusOrder",    AIInterface::IssueChangeFocusOrder);
    def("issueEnqueueTechOrder",    AIInterface::IssueEnqueueTechOrder);
    def("issueDequeueTechOrder",    AIInterface::IssueDequeueTechOrder);
    def("issueEnqueueBuildingProductionOrder",  AIInterface::IssueEnqueueBuildingProductionOrder);
    def("issueEnqueueShipProductionOrder",      AIInterface::IssueEnqueueShipProductionOrder);
    def("issueRequeueProductionOrder",          AIInterface::IssueRequeueProductionOrder);
    def("issueDequeueProductionOrder",          AIInterface::IssueDequeueProductionOrder);

    def("sendChatMessage",          AIInterface::SendPlayerChatMessage);

    def("setSaveStateString",       SetStaticSaveStateString);
    def("getSaveStateString",       GetStaticSaveStateString,       return_value_policy<copy_const_reference>());

    def("doneTurn",                 AIInterface::DoneTurn);


    ///////////////////
    //     Empire    //
    ///////////////////
    class_<Empire, noncopyable>("empire", no_init)
        .add_property("name",                   make_function(&Empire::Name,                    return_value_policy<copy_const_reference>()))
        .add_property("playerName",             make_function(&Empire::PlayerName,              return_value_policy<copy_const_reference>()))

        .add_property("empireID",               &Empire::EmpireID)
        .add_property("homeworldID",            &Empire::HomeworldID)
        .add_property("capitolID",              &Empire::CapitolID)

        .def("buildingTypeAvailable",           &Empire::BuildingTypeAvailable)
        .add_property("availableBuildingTypes", make_function(&Empire::AvailableBuildingTypes,  return_internal_reference<>()))
        .def("shipDesignAvailable",             &Empire::ShipDesignAvailable)
        .add_property("availableShipDesigns",   make_function(&Empire::AvailableShipDesigns,    return_value_policy<return_by_value>()))
        .add_property("productionQueue",        make_function(&Empire::GetProductionQueue,      return_internal_reference<>()))

        .def("techResearched",                  &Empire::TechResearched)
        .add_property("availableTechs",         make_function(&Empire::AvailableTechs,          return_internal_reference<>()))
        .def("getTechStatus",                   &Empire::GetTechStatus)
        .def("researchStatus",                  &Empire::ResearchStatus)
        .add_property("researchQueue",          make_function(&Empire::GetResearchQueue,        return_internal_reference<>()))

        .def("canBuild",                        BuildableItemBuildingOrbital)
        .def("canBuild",                        BuildableItemShip)

        .def("hasExploredSystem",               &Empire::HasExploredSystem)
        .add_property("exploredSystemIDs",      make_function(&Empire::ExploredSystems,         return_internal_reference<>()))
    ;

    ////////////////////
    // Research Queue //
    ////////////////////
    class_<ResearchQueue::Element>("researchQueueElement", no_init)
        .add_property("tech",                   make_getter(&ResearchQueue::Element::tech,      return_value_policy<reference_existing_object>()))
        .def_readonly("spending",               &ResearchQueue::Element::spending)
        .def_readonly("turnsLeft",              &ResearchQueue::Element::turns_left)
    ;
    class_<ResearchQueue, noncopyable>("researchQueue", no_init)
        .def("__iter__",                        iterator<ResearchQueue>())  // ResearchQueue provides STL container-like interface to contained queue
        .def("__getitem__",                     &ResearchQueue::operator[],                     return_internal_reference<>())
        .def("__len__",                         &ResearchQueue::size)
        .def("size",                            &ResearchQueue::size)
        .def("empty",                           &ResearchQueue::empty)
        .def("inQueue",                         &ResearchQueue::InQueue)
        .def("inQueue",                         make_function(
                                                    boost::bind(&ResearchQueue::InQueue, _1, boost::bind(&GetTech, _2)),
                                                    return_value_policy<return_by_value>(),
                                                    boost::mpl::vector<bool, const ResearchQueue*, const std::string&>()
                                                ))
        .def("__contains__",                    make_function(
                                                    boost::bind(InQueueFromResearchQueueElementFunc, _1, _2),
                                                    return_value_policy<return_by_value>(),
                                                    boost::mpl::vector<bool, const ResearchQueue*, const ResearchQueue::Element&>()
                                                ))
    ;

    //////////////////////
    // Production Queue //
    //////////////////////
    class_<ProductionQueue::Element>("productionQueueElement", no_init)
        .add_property("name",                   make_function(
                                                    boost::bind(NameFromProductionQueueElement, _1),
                                                    return_value_policy<copy_const_reference>(),
                                                    boost::mpl::vector<const std::string&, const ProductionQueue::Element&>()
                                                ))
        .add_property("designID",               make_function(
                                                    boost::bind(DesignIDFromProductionQueueElement, _1),
                                                    return_value_policy<return_by_value>(),
                                                    boost::mpl::vector<int, const ProductionQueue::Element&>()
                                                ))
        .add_property("buildType",              make_function(
                                                    boost::bind(BuildTypeFromProductionQueueElement, _1),
                                                    return_value_policy<return_by_value>(),
                                                    boost::mpl::vector<BuildType, const ProductionQueue::Element&>()
                                                ))
        .def_readonly("locationID",             &ProductionQueue::Element::location)
        .def_readonly("spending",               &ProductionQueue::Element::spending)
        .def_readonly("turnsLeft",              &ProductionQueue::Element::turns_left_to_completion)
        .add_property("totalSpent",             &ProductionQueue::TotalPPsSpent)
    ;
    class_<ProductionQueue, noncopyable>("productionQueue", no_init)
        .def("__iter__",                        iterator<ProductionQueue>())  // ProductionQueue provides STL container-like interface to contained queue
        .def("__getitem__",                     ProductionQueueOperatorSquareBrackets,          return_internal_reference<>())
        .def("__len__",                         &ProductionQueue::size)
        .def("size",                            &ProductionQueue::size)
        .def("empty",                           &ProductionQueue::empty)
    ;


    ////////////////////
    //    Universe    //
    ////////////////////
    class_<Universe, noncopyable>("universe", no_init)
        .def("getObject",                   UniverseGetObject,              return_value_policy<reference_existing_object>())
        .def("getFleet",                    UniverseGetFleet,               return_value_policy<reference_existing_object>())
        .def("getShip",                     UniverseGetShip,                return_value_policy<reference_existing_object>())
        .def("getPlanet",                   UniverseGetPlanet,              return_value_policy<reference_existing_object>())
        .def("getSystem",                   UniverseGetSystem,              return_value_policy<reference_existing_object>())
        .def("getBuilding",                 UniverseGetBuilding,            return_value_policy<reference_existing_object>())
        .def("getSpecial",                  GetSpecial,                     return_value_policy<reference_existing_object>())

        .add_property("allObjectIDs",       make_function(&Universe::FindObjectIDs<UniverseObject>, return_value_policy<return_by_value>()))
        .add_property("systemIDs",          make_function(&Universe::FindObjectIDs<System>,         return_value_policy<return_by_value>()))
        .add_property("fleetIDs",           make_function(&Universe::FindObjectIDs<Fleet>,          return_value_policy<return_by_value>()))

        .def("systemHasStarlane",           &Universe::SystemReachable)
        .def("systemsConnected",            &Universe::SystemsConnected)

        // put as part of universe class so one doesn't need a UniverseObject object in python to access these
        .def_readonly("invalidObjectID",    &UniverseObject::INVALID_OBJECT_ID)
        .def_readonly("invalidObjectAge",   &UniverseObject::INVALID_OBJECT_AGE)
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
        .add_property("owners",             make_function(&UniverseObject::Owners,      return_internal_reference<>()))
        .def("ownedBy",                     &UniverseObject::OwnedBy)
        .def("whollyOwnedBy",               &UniverseObject::WhollyOwnedBy)
        .add_property("creationTurn",       &UniverseObject::CreationTurn)
        .add_property("ageInTurns",         &UniverseObject::AgeInTurns)
        .add_property("specials",           make_function(&UniverseObject::Specials,    return_internal_reference<>()))
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
        .add_property("shipIDs",                    make_function(&Fleet::ShipIDs,      return_internal_reference<>()))
    ;

    //////////////////
    //     Ship     //
    //////////////////
    class_<Ship, bases<UniverseObject>, noncopyable>("ship", no_init)
        .add_property("design",             make_function(&Ship::Design,        return_value_policy<reference_existing_object>()))
        .add_property("fleetID",            &Ship::FleetID)
        .add_property("getFleet",           make_function(&Ship::GetFleet,      return_value_policy<reference_existing_object>()))
        .add_property("isArmed",            &Ship::IsArmed)
        .add_property("speed",              &Ship::Speed)
    ;

    //////////////////
    //  ShipDesign  //
    //////////////////
    class_<ShipDesign, noncopyable>("shipDesign", no_init)
        .add_property("name",               make_function(&ShipDesign::Name,    return_value_policy<copy_const_reference>()))
    ;

    //////////////////
    //   Building   //
    //////////////////
    class_<Building, bases<UniverseObject>, noncopyable>("building", no_init)
        .def("getBuildingType",             &Building::GetBuildingType,         return_value_policy<reference_existing_object>())
        .add_property("operating",          &Building::Operating)
        .def("getPlanet",                   &Building::GetPlanet,               return_value_policy<reference_existing_object>())
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
        .add_property("buildingIDs",        make_function(&Planet::Buildings,   return_internal_reference<>()))
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
        .add_property("allObjectIDs",       make_function(&System::FindObjectIDs<UniverseObject>,   return_value_policy<return_by_value>()))
        .add_property("planetIDs",          make_function(&System::FindObjectIDs<Planet>,           return_value_policy<return_by_value>()))
        .add_property("fleetIDs",           make_function(&System::FindObjectIDs<Fleet>,            return_value_policy<return_by_value>()))
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
        .add_property("prerequisites",      make_function(&Tech::Prerequisites,     return_internal_reference<>()))
        .add_property("unlockedTechs",      make_function(&Tech::UnlockedTechs,     return_internal_reference<>()))
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
    enum_<BuildType>("buildType")
        .value("unresearchable",    BT_BUILDING)
        .value("researchable",      BT_SHIP)
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

    FreeOrionPython::SetWrapper<int>::Wrap("IntSet");
    FreeOrionPython::SetWrapper<std::string>::Wrap("StringSet");
}
 
///////////////////////
//     PythonAI      //
///////////////////////
static dict         s_main_namespace = dict();
static object       s_ai_module = object();
static PythonAI*    s_ai = 0;
PythonAI::PythonAI() {
    // in order to expose a getter for it to Python, s_save_state_string must be static, and not a member
    // variable of class PythonAI, because the exposing is done outside the PythonAI class and there is no
    // access to a pointer to PythonAI
    if (s_ai)
        throw std::runtime_error("Attempted to create more than one Python AI instance");

    s_ai = this;

    Py_Initialize();    // initializes Python interpreter, allowing Python functions to be called from C++

    initfreeOrionLogger();          // allows the "freeOrionLogger" C++ module to be imported within Python code
    initfreeOrionAIInterface();     // allows the "freeOrionAIInterface" C++ module to be imported within Python code

    try {
        // get main namespace, needed to run other interpreted code
        object main_module = import("__main__");
        s_main_namespace = extract<dict>(main_module.attr("__dict__"));
    } catch (error_already_set err) {
        Logger().errorStream() << "Unable to initialize Python interpreter.";
        return;
    }

    try {
        // set up Logging by redirecting stdout and stderr to exposed logging functions
        std::string logger_script = "import sys\n"
                                    "import freeOrionLogger\n"
                                    "class debugLogger:\n"
                                    "  def write(self, stng):\n"
                                    "    freeOrionLogger.log(stng)\n"
                                    "class errorLogger:\n"
                                    "  def write(self, stng):\n"
                                    "    freeOrionLogger.error(stng)\n"
                                    "sys.stdout = debugLogger()\n"
                                    "sys.stderr = errorLogger()\n"
                                    "print 'Python stdout and stderr redirected'";
        object ignored = exec(logger_script.c_str(), s_main_namespace, s_main_namespace);
    } catch (error_already_set err) {
        Logger().errorStream() << "Unable to redirect Python stdout and stderr.";
        return;
    }

    try {
        // tell Python the path in which to locate AI script file
        std::string AI_path = (GetGlobalDir() / "default" / "AI").native_directory_string();
        std::string path_command = "sys.path.append('" + AI_path + "')";
        object ignored = exec(path_command.c_str(), s_main_namespace, s_main_namespace);

        // import AI script file and run initialization function
        s_ai_module = import("FreeOrionAI");
        object initAIPythonFunction = s_ai_module.attr("initFreeOrionAI");
        initAIPythonFunction();

        //ignored = exec(fo_interface_import_script.c_str(), s_main_namespace, s_main_namespace);
    } catch (error_already_set err) {
        PyErr_Print();
        return;
    }

    Logger().debugStream() << "Initialized Python AI";
}

PythonAI::~PythonAI() {
    Logger().debugStream() << "Cleaning up / destructing Python AI";
    Py_Finalize();      // stops Python interpreter and release its resources
    s_ai = 0;
    s_main_namespace = dict();
    s_ai_module = object();
}

void PythonAI::GenerateOrders() {
    AIInterface::InitTurn();
    try {
        // call Python function that generates orders for current turn
        object generateOrdersPythonFunction = s_ai_module.attr("generateOrders");
        generateOrdersPythonFunction();
    } catch (error_already_set err) {
        PyErr_Print();
        AIInterface::DoneTurn();
    }
}

void PythonAI::HandleChatMessage(int sender_id, const std::string& msg) {
    try {
        // call Python function that responds or ignores a chat message
        object handleChatMessagePythonFunction = s_ai_module.attr("handleChatMessage");
        handleChatMessagePythonFunction(sender_id, msg);
    } catch (error_already_set err) {
        PyErr_Print();
    }
}

void PythonAI::StartNewGame() {
    s_save_state_string = "";
    try {
        // call Python function that sets up the AI to be able to generate orders for a new game
        object startNewGamePythonFunction = s_ai_module.attr("startNewGame");
        startNewGamePythonFunction();
    } catch (error_already_set err) {
        PyErr_Print();
    }
}

void PythonAI::ResumeLoadedGame(const std::string& save_state_string) {
    //Logger().debugStream() << "PythonAI::ResumeLoadedGame(" << save_state_string << ")";
    s_save_state_string = save_state_string;
    try {
        // call Python function that deals with the new state string sent by the server
        object resumeLoadedGamePythonFunction = s_ai_module.attr("resumeLoadedGame");
        resumeLoadedGamePythonFunction(s_save_state_string);
    } catch (error_already_set err) {
        PyErr_Print();
    }
}

const std::string& PythonAI::GetSaveStateString() {
    try {
        // call Python function that serializes AI state for storage in save file and sets s_save_state_string
        // to contain that string
        object prepareForSavePythonFunction = s_ai_module.attr("prepareForSave");
        prepareForSavePythonFunction();
    } catch (error_already_set err) {
        PyErr_Print();
    }
    //Logger().debugStream() << "PythonAI::GetSaveStateString() returning: " << s_save_state_string;
    return s_save_state_string;
}
