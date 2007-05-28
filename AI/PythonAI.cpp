#include "PythonAI.h"
#include "../util/AppInterface.h"
#include "../util/Directories.h"

#include "../Empire/Empire.h"

#include <boost/python.hpp>

////////////////////////
// Python AIInterface //
////////////////////////
// disambiguate overloaded AIInterface functions
const std::string& (*AIIntPlayerNameVoid)(void) = &AIInterface::PlayerName;
const std::string& (*AIIntPlayerNameInt)(int) = &AIInterface::PlayerName;

const Empire* (*AIIntGetEmpireVoid)(void) = &AIInterface::GetEmpire;
const Empire* (*AIIntGetEmpireInt)(int) = &AIInterface::GetEmpire;


// expose to Python
BOOST_PYTHON_MODULE(foaiint)
{
    using boost::python::def;
    using boost::python::return_value_policy;
    using boost::python::copy_const_reference;
    using boost::python::reference_existing_object;
    using boost::python::class_;
    using boost::noncopyable;
    using boost::python::no_init;

    // AIInterface
    def("PlayerName",               AIIntPlayerNameVoid,        return_value_policy<copy_const_reference>());
    def("IDPlayerName",             AIIntPlayerNameInt,         return_value_policy<copy_const_reference>());

    def("PlayerID",                 AIInterface::PlayerID);
    def("EmpireID",                 AIInterface::EmpireID);
    def("EmpirePlayerID",           AIInterface::EmpirePlayerID);

    def("GetEmpire",                AIIntGetEmpireVoid,         return_value_policy<reference_existing_object>());
    def("GetIDEmpire",              AIIntGetEmpireInt,          return_value_policy<reference_existing_object>());

    def("CurrentTurn",              AIInterface::CurrentTurn);

    def("IssueFleetMoveOrder",      AIInterface::IssueFleetMoveOrder);
    def("IssueRenameOrder",         AIInterface::IssueRenameOrder);
    def("IssueNewFleetOrder",       AIInterface::IssueNewFleetOrder);
    def("IssueFleetColonizeOrder",  AIInterface::IssueFleetColonizeOrder);

    def("SendChatMessage",          AIInterface::SendPlayerChatMessage);

    def("DoneTurn",                 AIInterface::DoneTurn);

    def("LogOutput",                AIInterface::LogOutput);

    // Empire
    class_<Empire, noncopyable>("Empire", no_init)
        .def("Name",                    &Empire::Name,          return_value_policy<copy_const_reference>())
        .def("PlayerName",              &Empire::PlayerName,    return_value_policy<copy_const_reference>())
        .def("EmpireID", &Empire::EmpireID)
        .def("HomeworldID",             &Empire::HomeworldID)
        .def("CapitolID",               &Empire::CapitolID)
        .def("BuildingTypeAvailable",   &Empire::BuildingTypeAvailable)
    ;
}
 
///////////////////////
//     PythonAI      //
///////////////////////
PythonAI::PythonAI()
{
    using boost::python::borrowed;

    Py_Initialize();    // initializes Python interpreter, allowing Python functions to be called from C++

    initfoaiint();   // allows the "foaiint" C++ module to be imported within Python code

    
    // get access to Python main namespace, which is needed to call other functions below
    try {
        main_module = PyOBJECT((PyHANDLE(borrowed(PyImport_AddModule("__main__")))));
        dict = PyOBJECT(main_module.attr("__dict__"));
    } catch (PyERROR err) {
        Logger().errorStream() << "error with main module or namespace";
        return;
    }

    PyHANDLE handle;

    // import Python built-in sys module, giving access to sys.path below
    try {
        handle = PyHANDLE(PyRun_String("import sys", Py_file_input, dict.ptr(), dict.ptr()));
    } catch (PyERROR err) {
        Logger().errorStream() << "error importing sys";
        return;
    }

    // tell Python the path in which to locate AI script file
    std::string AI_path = (GetGlobalDir() / "default" / "AI").native_directory_string();
    std::string python_path_command = "sys.path.append('" + AI_path + "')";
    try {
        handle = PyHANDLE(PyRun_String(python_path_command.c_str(), Py_file_input, dict.ptr(), dict.ptr()));
    } catch (PyERROR err) {
        Logger().errorStream() << "error appending to sys.path";
        return;
    }

    // load Python script of AI functions
    try {
        handle = PyHANDLE(PyRun_String("import FreeOrionAI", Py_file_input, dict.ptr(), dict.ptr()));
    } catch (PyERROR err) {
        Logger().errorStream() << "error importing FreeOrionAI.py into Python";
        return;
    }

    // initialize AI within Python
    try {
        handle = PyHANDLE(PyRun_String("FreeOrionAI.InitFreeOrionAI()", Py_file_input, dict.ptr(), dict.ptr()));
    } catch(PyERROR err) {
        Logger().errorStream() << "error calling FreeOrionAI.InitFreeOrionAI()";
        return;
    }

    Logger().debugStream() << "Initialized PythonAI";
}

PythonAI::~PythonAI()
{
    std::cout << "Cleaning up / destructing Python AI" << std::endl;
    Py_Finalize();      // stops Python interpreter and release its resources
}

void PythonAI::GenerateOrders()
{
    try {
        PyHANDLE handle = PyHANDLE(PyRun_String("FreeOrionAI.GenerateOrders()", Py_file_input, dict.ptr(), dict.ptr()));
    } catch(PyERROR err) {
        Logger().errorStream() << "error calling FreeOrionAI.GenerateOrders()";
        AIInterface::DoneTurn();
        return;
    }
}

void PythonAI::HandleChatMessage(int sender_id, const std::string& msg)
{}
