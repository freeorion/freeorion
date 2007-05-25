#include "PythonAI.h"

#include <boost/python.hpp>

////////////////////////
// Python AIInterface //
////////////////////////
BOOST_PYTHON_MODULE(FreeOrionAIPythonInterface)
{
    boost::python::def("EmpirePlayerID", AIInterface::EmpirePlayerID);
    boost::python::def("CurrentTurn", AIInterface::CurrentTurn);

    boost::python::def("DoneTurn", AIInterface::DoneTurn);
}
 
///////////////////////
//     PythonAI      //
///////////////////////
PythonAI::PythonAI() :
    AIBase()
{
    std::cout << "Starting Python AI" << std::endl;

    Py_Initialize();    // initializes Python interpreter, allowing Python functions to be called from C++

    initFreeOrionAIPythonInterface();   // allows the "FreeOrionAIPythonInterface" C++ module to be imported within Python code

    /* import contents of "FreeOrionAI.py" into Python */
    int ignored = PyRun_SimpleString("import FreeOrionAI");
}

PythonAI::~PythonAI()
{
    std::cout << "Cleaning up / destructing Python AI" << std::endl;
    Py_Finalize();      // stops Python interpreter and release its resources
}

void PythonAI::GenerateOrders()
{
    /* call the GenerateOrders() Python function, which is defined in "FreeOrionAI.py" */
    int ignored = PyRun_SimpleString("FreeOrionAI.GenerateOrders()");
}

void PythonAI::HandleChatMessage(int sender_id, const std::string& msg)
{}
