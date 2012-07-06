#include "PythonAI.h"

#include "../util/MultiplayerCommon.h"
#include "../util/AppInterface.h"
#include "../util/Directories.h"
#include "../Empire/Empire.h"
#include "../Empire/Diplomacy.h"
#include "../python/PythonWrappers.h"

#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include <boost/python/suite/indexing/map_indexing_suite.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/timer.hpp>

using boost::python::class_;
using boost::python::def;
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


////////////////////////
// Python AIInterface //
////////////////////////
// disambiguation of overloaded functions
const std::string&  (*AIIntPlayerNameVoid)(void) =              &AIInterface::PlayerName;
const std::string&  (*AIIntPlayerNameInt)(int) =                &AIInterface::PlayerName;

const Empire*       (*AIIntGetEmpireVoid)(void) =               &AIInterface::GetEmpire;
const Empire*       (*AIIntGetEmpireInt)(int) =                 &AIInterface::GetEmpire;

int                 (*AIIntNewFleet)(const std::string&, int) = &AIInterface::IssueNewFleetOrder;
int                 (*AIIntScrap)(int)                        = &AIInterface::IssueScrapOrder;


namespace {
    // static s_save_state_string, getter and setter to be exposed to Python
    static std::string s_save_state_string("");

    static const std::string& GetStaticSaveStateString()
    { return s_save_state_string; }

    static void SetStaticSaveStateString(const std::string& new_state_string)
    { s_save_state_string = new_state_string; }
}


// Create the freeOrionLogger Python module, which exposes debug and error (stdout and stderr respectively)
// sinks so Python text output can be recovered and saved in c++.  These can be accessed from within Python
// by freeOrionLogger.log(stringParam) and freeOrionLogger.error(stringParam)
BOOST_PYTHON_MODULE(freeOrionLogger)
{
    FreeOrionPython::WrapLogger();
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
 * return_value_policy<reference_existing_object>   when returning an object from a non-member function
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

    def("issueFleetMoveOrder",                  AIInterface::IssueFleetMoveOrder);
    def("issueRenameOrder",                     AIInterface::IssueRenameOrder);
    def("issueScrapOrder",                      AIIntScrap);
    def("issueNewFleetOrder",                   AIIntNewFleet);
    def("issueFleetTransferOrder",              AIInterface::IssueFleetTransferOrder);
    def("issueColonizeOrder",                   AIInterface::IssueColonizeOrder);
    def("issueInvadeOrder",                     AIInterface::IssueInvadeOrder);
    def("issueAggressionOrder",                 AIInterface::IssueAggressionOrder);
    def("issueChangeFocusOrder",                AIInterface::IssueChangeFocusOrder);
    def("issueEnqueueTechOrder",                AIInterface::IssueEnqueueTechOrder);
    def("issueDequeueTechOrder",                AIInterface::IssueDequeueTechOrder);
    def("issueEnqueueBuildingProductionOrder",  AIInterface::IssueEnqueueBuildingProductionOrder);
    def("issueEnqueueShipProductionOrder",      AIInterface::IssueEnqueueShipProductionOrder);
    def("issueRequeueProductionOrder",          AIInterface::IssueRequeueProductionOrder);
    def("issueDequeueProductionOrder",          AIInterface::IssueDequeueProductionOrder);
    def("issueCreateShipDesignOrder",           AIInterface::IssueCreateShipDesignOrder);

    def("sendChatMessage",          AIInterface::SendPlayerChatMessage);
    def("sendDiplomaticMessage",    AIInterface::SendDiplomaticMessage);

    def("setSaveStateString",       SetStaticSaveStateString);
    def("getSaveStateString",       GetStaticSaveStateString,       return_value_policy<copy_const_reference>());

    def("doneTurn",                 AIInterface::DoneTurn);


    //////////////////
    //    Empire    //
    //////////////////
    FreeOrionPython::WrapEmpire();


    /////////////////////
    // UniverseClasses //
    /////////////////////
    FreeOrionPython::WrapUniverseClasses();


    ///////////////////
    //     Enums     //
    ///////////////////
    FreeOrionPython::WrapGameStateEnums();


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
 
//////////////////////
//     PythonAI     //
//////////////////////
static dict         s_main_namespace = dict();
static object       s_ai_module = object();
static PythonAI*    s_ai = 0;
PythonAI::PythonAI() {
    Logger().debugStream() << "PythonAI::PythonAI()";
    // in order to expose a getter for it to Python, s_save_state_string must be static, and not a member
    // variable of class PythonAI, because the exposing is done outside the PythonAI class and there is no
    // access to a pointer to PythonAI
    if (s_ai)
        throw std::runtime_error("Attempted to create more than one Python AI instance");

    s_ai = this;

    try {
#ifdef FREEORION_MACOSX
#include <sys/param.h>
        char python_home[MAXPATHLEN];
        strcpy(python_home, GetPythonHome().string().c_str());
        Py_SetPythonHome(python_home);
#endif
        Py_Initialize();                // initializes Python interpreter, allowing Python functions to be called from C++
        Logger().debugStream() << "Python initialized";

        Logger().debugStream() << "Python version: " << Py_GetVersion();
        Logger().debugStream() << "Python prefix: " << Py_GetPrefix();
        Logger().debugStream() << "Python module search path: " << Py_GetPath();

        Logger().debugStream() << "Initializing C++ interfaces for Python";

        initfreeOrionLogger();          // allows the "freeOrionLogger" C++ module to be imported within Python code
        initfreeOrionAIInterface();     // allows the "freeOrionAIInterface" C++ module to be imported within Python code
    } catch (...) {
        Logger().errorStream() << "Unable to initialize Python interpreter.";
        return;
    }

    try {
        // get main namespace, needed to run other interpreted code
        object main_module = import("__main__");
        s_main_namespace = extract<dict>(main_module.attr("__dict__"));
    } catch (error_already_set err) {
        Logger().errorStream() << "Unable to set up main namespace in Python.";
        PyErr_Print();
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
                                    "print ('Python stdout and stderr redirected')";
        object ignored = exec(logger_script.c_str(), s_main_namespace, s_main_namespace);
    } catch (error_already_set err) {
        Logger().errorStream() << "Unable to redirect Python stdout and stderr.";
        return;
    }

    try {
        // tell Python the path in which to locate AI script file
        std::string AI_path = (GetResourceDir() / "AI").string();
        std::string path_command = "sys.path.append(r'" + AI_path + "')";
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
    Logger().debugStream() << "PythonAI::GenerateOrders : initializing turn";
    AIInterface::InitTurn();

    boost::timer order_timer;
    try {
        // call Python function that generates orders for current turn
        //Logger().debugStream() << "PythonAI::GenerateOrders : getting generate orders object";
        object generateOrdersPythonFunction = s_ai_module.attr("generateOrders");
        //Logger().debugStream() << "PythonAI::GenerateOrders : generating orders";
        generateOrdersPythonFunction();
    } catch (error_already_set err) {
        PyErr_Print();
        //Logger().debugStream() << "PythonAI::GenerateOrders : python error caught and printed";
        AIInterface::DoneTurn();
        //Logger().debugStream() << "PythonAI::GenerateOrders : done with error";
    }
    Logger().debugStream() << "PythonAI::GenerateOrders order generating time: " << (order_timer.elapsed() * 1000.0);
}

void PythonAI::GenerateCombatSetupOrders(const CombatData& combat_data)
{ AIBase::GenerateCombatSetupOrders(combat_data); }

void PythonAI::GenerateCombatOrders(const CombatData& combat_data)
{ AIBase::GenerateCombatOrders(combat_data); }

void PythonAI::HandleChatMessage(int sender_id, const std::string& msg) {
    try {
        // call Python function that responds or ignores a chat message
        object handleChatMessagePythonFunction = s_ai_module.attr("handleChatMessage");
        handleChatMessagePythonFunction(sender_id, msg);
    } catch (error_already_set err) {
        PyErr_Print();
    }
}

void PythonAI::HandleDiplomaticMessage(const DiplomaticMessage& msg) {
    try {
        // call Python function to inform of diplomatic message change
        object handleDiplomaticMessagePythonFunction = s_ai_module.attr("handleDiplomaticMessage");
        handleDiplomaticMessagePythonFunction(msg);
    } catch (error_already_set err) {
        PyErr_Print();
    }
}

void PythonAI::HandleDiplomaticStatusUpdate(const DiplomaticStatusUpdateInfo& u) {
    try {
        // call Python function to inform of diplomatic status update
        object handleDiplomaticStatusUpdatePythonFunction = s_ai_module.attr("handleDiplomaticStatusUpdate");
        handleDiplomaticStatusUpdatePythonFunction(u);
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
