#include "AIFramework.h"

#include "AIClientApp.h"
#include "AIWrapper.h"
#include "../../universe/BuildingType.h"
#include "../../universe/Universe.h"
#include "../../util/Directories.h"
#include "../../util/Logger.h"
#include "../../util/i18n.h"
#include "../../util/OptionsDB.h"
#include "../../util/ScopedTimer.h"
#include "../../Empire/Empire.h"
#include "../../Empire/Diplomacy.h"
#include "../../python/SetWrapper.h"
#include "../../python/CommonWrappers.h"

#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include <boost/python/suite/indexing/map_indexing_suite.hpp>
#include <boost/python/docstring_options.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/python/list.hpp>
#include <boost/python/extract.hpp>
#include <boost/python/scope.hpp>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;
namespace py = boost::python;

namespace {
    struct string_view_to_python_str {
        static PyObject* convert(const std::string_view& s)
        { return boost::python::incref(boost::python::object(s.data()).ptr()); }
    };
}

BOOST_PYTHON_MODULE(freeOrionAIInterface)
{
    py::docstring_options doc_options(true, true, false);

    boost::python::to_python_converter<std::string_view, string_view_to_python_str>();


    ///////////////////
    //  Game client  //
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

    ///////////////////
    //     Config    //
    ///////////////////
    FreeOrionPython::WrapConfig();

    ////////////////////
    // STL Containers //
    ////////////////////
    py::class_<std::vector<int>>("IntVec")
        .def(py::vector_indexing_suite<std::vector<int>>())
    ;
    py::class_<std::vector<std::string>>("StringVec")
        .def(py::vector_indexing_suite<std::vector<std::string>>())
    ;

    py::class_<std::map<int, bool>>("IntBoolMap")
        .def(py::map_indexing_suite<std::map<int, bool>>())
    ;

    FreeOrionPython::SetWrapper<std::set<int>>::Wrap("IntSet");
    FreeOrionPython::SetWrapper<std::set<std::string>>::Wrap("StringSet");
}

//////////////////////
//     PythonAI     //
//////////////////////
auto PythonAI::Initialize() -> bool
{
    if (PythonBase::Initialize()) {
        BuildingTypeManager& temp = GetBuildingTypeManager();  // Ensure buildings are initialized
        (void)temp; // Hide unused variable warning
        return true;
    }
    else
        return false;
}

auto PythonAI::InitImports() -> bool
{
    DebugLogger() << "Initializing AI Python imports";
    // allows the "freeOrionAIInterface" C++ module to be imported within Python code
    return PyImport_AppendInittab("freeOrionAIInterface", &PyInit_freeOrionAIInterface) != -1;
}

auto PythonAI::InitModules() -> bool
{
    DebugLogger() << "Initializing AI Python modules";

    // Confirm existence of the directory containing the AI Python scripts
    // and add it to Pythons sys.path to make sure Python will find our scripts
    fs::path ai_path = fs::weakly_canonical(GetResourceDir() / FilenameToPath(GetOptionsDB().Get<std::string>("ai-path")));
    DebugLogger() << "AI Python script path: " << PathToString(ai_path);
    if (!fs::exists(ai_path)) {
        ErrorLogger() << "Can't find folder containing AI scripts";
        return false;
    }
    AddToSysPath(ai_path);
    DebugLogger() << "AI Python modules successfully initialized!";
    return true;
}

void PythonAI::Start()
{
    // import AI main file
    m_python_module_ai = py::import("FreeOrionAI");
    DebugLogger() << "AI Python modules started!";
}

void PythonAI::GenerateOrders()
{
    DebugLogger() << "PythonAI::GenerateOrders : initializing turn";

    ScopedTimer order_timer;
    try {
        // call Python function that generates orders for current turn
        //DebugLogger() << "PythonAI::GenerateOrders : getting generate orders object";
        py::object generateOrdersPythonFunction = m_python_module_ai.attr("generateOrders");
        //DebugLogger() << "PythonAI::GenerateOrders : generating orders";
        generateOrdersPythonFunction();
    } catch (const py::error_already_set&) {
        HandleErrorAlreadySet();
        if (!IsPythonRunning() || GetOptionsDB().Get<bool>("testing"))
            throw;

        ErrorLogger() << "PythonAI::GenerateOrders : Python error caught.  Partial orders sent to server";
    }

    AIClientApp* app = AIClientApp::GetApp();
    // encodes order sets and sends turn orders message.  "done" the turn for the client, but "starts" the turn for the server
    app->StartTurn(app->GetAI()->GetSaveStateString());

    DebugLogger() << "PythonAI::GenerateOrders order generating time: " << order_timer.DurationString();
}

void PythonAI::HandleChatMessage(int sender_id, const std::string& msg)
{
    // call Python function that responds or ignores a chat message
    py::object handleChatMessagePythonFunction = m_python_module_ai.attr("handleChatMessage");
    handleChatMessagePythonFunction(sender_id, msg);
}

void PythonAI::HandleDiplomaticMessage(const DiplomaticMessage& msg)
{
    // call Python function to inform of diplomatic message change
    py::object handleDiplomaticMessagePythonFunction = m_python_module_ai.attr("handleDiplomaticMessage");
    handleDiplomaticMessagePythonFunction(msg);
}

void PythonAI::HandleDiplomaticStatusUpdate(const DiplomaticStatusUpdateInfo& u)
{
    // call Python function to inform of diplomatic status update
    py::object handleDiplomaticStatusUpdatePythonFunction = m_python_module_ai.attr("handleDiplomaticStatusUpdate");
    handleDiplomaticStatusUpdatePythonFunction(u);
}

void PythonAI::StartNewGame()
{
    FreeOrionPython::ClearStaticSaveStateString();
    // call Python function that sets up the AI to be able to generate orders for a new game
    py::object startNewGamePythonFunction = m_python_module_ai.attr("startNewGame");
    startNewGamePythonFunction();
}

void PythonAI::ResumeLoadedGame(const std::string& save_state_string)
{
    //DebugLogger() << "PythonAI::ResumeLoadedGame(" << save_state_string << ")";
    FreeOrionPython::SetStaticSaveStateString(save_state_string);
    // call Python function that deals with the new state string sent by the server
    py::object resumeLoadedGamePythonFunction = m_python_module_ai.attr("resumeLoadedGame");
    resumeLoadedGamePythonFunction(FreeOrionPython::GetStaticSaveStateString());
}

auto PythonAI::GetSaveStateString() const -> const std::string&
{
    // call Python function that serializes AI state for storage in save file and sets s_save_state_string
    // to contain that string
    py::object prepareForSavePythonFunction = m_python_module_ai.attr("prepareForSave");
    prepareForSavePythonFunction();
    //DebugLogger() << "PythonAI::GetSaveStateString() returning: " << s_save_state_string;
    return FreeOrionPython::GetStaticSaveStateString();
}
