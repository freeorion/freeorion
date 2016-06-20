#include "AIFramework.h"

#include "../../universe/Building.h"
#include "../../universe/Universe.h"
#include "../../util/Directories.h"
#include "../../util/Logger.h"
#include "../../util/i18n.h"
#include "../../util/MultiplayerCommon.h"
#include "../../util/OptionsDB.h"
#include "../../Empire/Empire.h"
#include "../../Empire/EmpireManager.h"
#include "../../Empire/Diplomacy.h"
#include "../CommonFramework.h"
#include "../SetWrapper.h"
#include "../CommonWrappers.h"
#include "AIWrapper.h"

#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include <boost/python/suite/indexing/map_indexing_suite.hpp>
#include <boost/python/docstring_options.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/timer.hpp>
#include <boost/python/list.hpp>
#include <boost/python/extract.hpp>
#include <boost/python/scope.hpp>
#include <boost/filesystem.hpp>

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
using boost::python::list;
using boost::python::extract;

namespace fs = boost::filesystem;


BOOST_PYTHON_MODULE(freeOrionAIInterface)
{
    boost::python::docstring_options doc_options(true, true, false);

    ///////////////////
    //  AIInterface  //
    ///////////////////
    FreeOrionPython::WrapAI();

    //////////////////
    //    Empire    //
    //////////////////
    FreeOrionPython::WrapEmpire();

    /////////////////////
    // UniverseClasses //
    /////////////////////
    FreeOrionPython::WrapUniverseClasses();
    FreeOrionPython::WrapGalaxySetupData();

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

    class_<std::map<int, bool> >("IntBoolMap")
        .def(map_indexing_suite<std::map<int, bool> >())
    ;

    FreeOrionPython::SetWrapper<int>::Wrap("IntSet");
    FreeOrionPython::SetWrapper<std::string>::Wrap("StringSet");
}
 
//////////////////////
//     PythonAI     //
//////////////////////
bool PythonAI::Initialize() {
    if (PythonBase::Initialize()) {
        try {
            BuildingTypeManager& temp = GetBuildingTypeManager();  // Ensure buildings are initialized
        } catch (error_already_set err) {
            PyErr_Print();
            return false;
        }
        return true;
    }
    else
        return false;
}

bool PythonAI::InitModules() {
    DebugLogger() << "Initializing AI Python modules";

    // allows the "freeOrionAIInterface" C++ module to be imported within Python code
    try {
        initfreeOrionAIInterface();
    } catch (...) {
        ErrorLogger() << "Unable to initialize 'freeOrionAIInterface' AI Python module";
        return false;
    }

    // Confirm existence of the directory containing the AI Python scripts
    // and add it to Pythons sys.path to make sure Python will find our scripts
    std::string ai_path = (GetResourceDir() / GetOptionsDB().Get<std::string>("ai-path")).string();
    DebugLogger() << "AI Python script path: " << ai_path;
    if (!fs::exists(ai_path)) {
        ErrorLogger() << "Can't find folder containing AI scripts";
        return false;
    }
    if (!AddToSysPath(ai_path)) {
        ErrorLogger() << "Can't add folder containing AI scripts to sys.path";
        return false;
    }

    try {
        // import universe generator script file
        m_python_module_ai = import("FreeOrionAI");
    }
    catch (error_already_set err) {
        ErrorLogger() << "Unable to import AI script";
        PyErr_Print();
        return false;
    }

    DebugLogger() << "AI Python modules successfully initialized!";
    return true;
}

void PythonAI::GenerateOrders() {
    DebugLogger() << "PythonAI::GenerateOrders : initializing turn";
    //AIInterface::InitTurn();

    boost::timer order_timer;
    try {
        // call Python function that generates orders for current turn
        //DebugLogger() << "PythonAI::GenerateOrders : getting generate orders object";
        object generateOrdersPythonFunction = m_python_module_ai.attr("generateOrders");
        //DebugLogger() << "PythonAI::GenerateOrders : generating orders";
        generateOrdersPythonFunction();
    } catch (error_already_set err) {
        PyErr_Print();
        //DebugLogger() << "PythonAI::GenerateOrders : python error caught and printed";
        AIInterface::DoneTurn();
        //DebugLogger() << "PythonAI::GenerateOrders : done with error";
    }
    DebugLogger() << "PythonAI::GenerateOrders order generating time: " << (order_timer.elapsed() * 1000.0);
}

void PythonAI::HandleChatMessage(int sender_id, const std::string& msg) {
    try {
        // call Python function that responds or ignores a chat message
        object handleChatMessagePythonFunction = m_python_module_ai.attr("handleChatMessage");
        handleChatMessagePythonFunction(sender_id, msg);
    } catch (error_already_set err) {
        PyErr_Print();
    }
}

void PythonAI::HandleDiplomaticMessage(const DiplomaticMessage& msg) {
    try {
        // call Python function to inform of diplomatic message change
        object handleDiplomaticMessagePythonFunction = m_python_module_ai.attr("handleDiplomaticMessage");
        handleDiplomaticMessagePythonFunction(msg);
    } catch (error_already_set err) {
        PyErr_Print();
    }
}

void PythonAI::HandleDiplomaticStatusUpdate(const DiplomaticStatusUpdateInfo& u) {
    try {
        // call Python function to inform of diplomatic status update
        object handleDiplomaticStatusUpdatePythonFunction = m_python_module_ai.attr("handleDiplomaticStatusUpdate");
        handleDiplomaticStatusUpdatePythonFunction(u);
    } catch (error_already_set err) {
        PyErr_Print();
    }
}

void PythonAI::StartNewGame() {
    FreeOrionPython::ClearStaticSaveStateString();
    try {
        // call Python function that sets up the AI to be able to generate orders for a new game
        object startNewGamePythonFunction = m_python_module_ai.attr("startNewGame");
        startNewGamePythonFunction(m_aggression);
    } catch (error_already_set err) {
        PyErr_Print();
    }
}

void PythonAI::ResumeLoadedGame(const std::string& save_state_string) {
    //DebugLogger() << "PythonAI::ResumeLoadedGame(" << save_state_string << ")";
    FreeOrionPython::SetStaticSaveStateString(save_state_string);
    try {
        // call Python function that deals with the new state string sent by the server
        object resumeLoadedGamePythonFunction = m_python_module_ai.attr("resumeLoadedGame");
        resumeLoadedGamePythonFunction(FreeOrionPython::GetStaticSaveStateString());
    } catch (error_already_set err) {
        PyErr_Print();
    }
}

const std::string& PythonAI::GetSaveStateString() {
    try {
        // call Python function that serializes AI state for storage in save file and sets s_save_state_string
        // to contain that string
        object prepareForSavePythonFunction = m_python_module_ai.attr("prepareForSave");
        prepareForSavePythonFunction();
    } catch (error_already_set err) {
        PyErr_Print();
    }
    //DebugLogger() << "PythonAI::GetSaveStateString() returning: " << s_save_state_string;
    return FreeOrionPython::GetStaticSaveStateString();
}
