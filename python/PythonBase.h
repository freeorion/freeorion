#ifndef __FreeOrion__Python__PythonBase__
#define __FreeOrion__Python__PythonBase__

#include "../util/boost_fix.h"
#include <boost/optional.hpp>
#include <boost/python.hpp>
#include <boost/python/dict.hpp>

#include <vector>
#include <string>

#include "../util/PythonCommon.h"

namespace boost::filesystem { class path; }

class PythonBase : public PythonCommon {
public:
    PythonBase() = default;
    ~PythonBase();

    bool         Initialize();                         // initializes and runs the Python interpreter, prepares the Python environment
    virtual bool InitCommonImports() override;         // initializes Python imports, must be implemented by derived classes
    virtual bool InitImports() = 0;                    // initializes Python imports, must be implemented by derived classes
    virtual bool InitModules() = 0;                    // initializes Python modules, must be implemented by derived classes
    void         SetCurrentDir(const boost::filesystem::path& dir); // sets Python current work directory or throws error_already_set
    void         AddToSysPath(const boost::filesystem::path& dir);  // adds directory to Python sys.path or throws error_already_set
    void         SetErrorModule(boost::python::object& module);     // sets Python module that contains error report function defined on the Python side

    std::vector<std::string> ErrorReport();            // wraps call to error report function defined on the Python side

private:
    void         Finalize();                           // stops Python interpreter and releases its resources

    boost::optional<boost::python::dict> m_namespace;           // stores main namespace in optional to be finalized before Python interpreter
    boost::python::object*  m_python_module_error = nullptr;    // used to track if and which Python module contains the "error_report" function ErrorReport should call
};

// returns root folder containing all the Python scripts
boost::filesystem::path GetPythonDir();

// returns folder containing common Python modules used by all Python scripts
boost::filesystem::path GetPythonCommonDir();


#endif /* defined(__FreeOrion__Python__PythonBase__) */
