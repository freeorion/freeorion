#include "AIInterface.h"

#include <boost/python.hpp>

#include <string>

class PythonAI : public AIBase
{
public:
    /** \name structors */ //@{
    PythonAI();
    ~PythonAI();
    //@}

    void GenerateOrders();
    void HandleChatMessage(int sender_id, const std::string& msg);

private:
    typedef boost::python::handle<PyObject>     PyHANDLE;
    typedef boost::python::object               PyOBJECT;
    typedef boost::python::error_already_set    PyERROR;

    PyOBJECT main_module;
    PyOBJECT dict;
};
