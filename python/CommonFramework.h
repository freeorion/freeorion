#ifndef __FreeOrion__Python__CommonFramework__
#define __FreeOrion__Python__CommonFramework__

#include <boost/python.hpp>
#include <boost/python/dict.hpp>

#include <vector>
#include <string>


class PythonBase {
public:
    /** \name ctor */ //@{
    PythonBase();
    virtual ~PythonBase();
    //@}

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
    void         Finalize();                           // stops Python interpreter and releases its resources
    virtual bool InitModules() = 0;                    // initializes Python modules, must be implemented by derived classes
    void         SetCurrentDir(const std::string dir); // sets Python current work directory or throws error_already_set
    void         AddToSysPath(const std::string dir);  // adds directory to Python sys.path or throws error_already_set
    void         SetErrorModule(boost::python::object& module); // sets Python module that contains error report function defined on the Python side

    std::vector<std::string> ErrorReport();            // wraps call to error report function defined on the Python side

private:
    // A copy of the systemExit exception to compare with returned
    // exceptions.  It can't be created in the exception handler.
    boost::python::object   m_system_exit;

    // some helper objects needed to initialize and run the Python interface
#if defined(FREEORION_MACOSX) || defined(FREEORION_WIN32)
    char                    m_home_dir[1024];
    char                    m_program_name[1024];
#endif
    boost::python::dict     m_namespace;
    boost::python::object*  m_python_module_error;  // used to track if and which Python module contains the "error_report" function ErrorReport should call
};

// returns root folder containing all the Python scripts
const std::string GetPythonDir();

// returns folder containing common Python modules used by all Python scripts
const std::string GetPythonCommonDir();


#endif /* defined(__FreeOrion__Python__CommonFramework__) */
