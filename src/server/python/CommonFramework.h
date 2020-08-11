#ifndef __FreeOrion__Python__CommonFramework__
#define __FreeOrion__Python__CommonFramework__

#include <boost/optional.hpp>
#include <boost/python.hpp>
#include <boost/python/dict.hpp>

#include <vector>
#include <string>


class PythonBase {
public:
    PythonBase();
    virtual ~PythonBase();

    /**
       Handles boost::python::error_already_set.
       If the error is SystemExit the python interpreter is finalized
       and no longer available.

       Call PyErr_Print() if the exception is an error.

       HandleErrorAlreadySet is idempotent, calling it multiple times
       won't crash or hang the process.
     */
    void HandleErrorAlreadySet();

    /**
       IsPythonRunning returns true is the python interpreter is
       initialized.  It is typically called after
       HandleErrorAlreadySet() to determine if the error caused the
       interpreter to shutdown.
     */
    bool IsPythonRunning();

    bool         Initialize();                         // initializes and runs the Python interpreter, prepares the Python environment
    virtual bool InitImports() = 0;                    // initializes Python imports, must be implemented by derived classes
    virtual bool InitModules() = 0;                    // initializes Python modules, must be implemented by derived classes
    void         SetCurrentDir(const std::string dir); // sets Python current work directory or throws error_already_set
    void         AddToSysPath(const std::string dir);  // adds directory to Python sys.path or throws error_already_set
    void         SetErrorModule(boost::python::object& module); // sets Python module that contains error report function defined on the Python side

    std::vector<std::string> ErrorReport();            // wraps call to error report function defined on the Python side

private:
    void         Finalize();                           // stops Python interpreter and releases its resources
    // A copy of the systemExit exception to compare with returned
    // exceptions.  It can't be created in the exception handler.
    boost::python::object   m_system_exit;

    // some helper objects needed to initialize and run the Python interface
#if defined(FREEORION_MACOSX) || defined(FREEORION_WIN32)
    wchar_t*                m_home_dir = nullptr;
    wchar_t*                m_program_name = nullptr;
#endif
    boost::optional<boost::python::dict> m_namespace;           // stores main namespace in optional to be finalized before Python interpreter
    boost::python::object*  m_python_module_error = nullptr;    // used to track if and which Python module contains the "error_report" function ErrorReport should call
};

// returns root folder containing all the Python scripts
const std::string GetPythonDir();

// returns folder containing common Python modules used by all Python scripts
const std::string GetPythonCommonDir();


#endif /* defined(__FreeOrion__Python__CommonFramework__) */
