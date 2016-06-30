#ifndef __FreeOrion__Python__CommonFramework__
#define __FreeOrion__Python__CommonFramework__

#include <boost/python.hpp>
#include <boost/python/tuple.hpp>
#include <boost/python/list.hpp>
#include <boost/python/dict.hpp>
#include <boost/python/extract.hpp>

#include <vector>
#include <string>


class PythonBase {
public:
    /** \name ctor */ //@{
    PythonBase();
    virtual ~PythonBase();
    //@}

    bool         Initialize();                         // initializes and runs the Python interpreter, prepares the Python environment
    void         Finalize();                           // stops Python interpreter and releases its resources
    virtual bool InitModules() = 0;                    // initializes Python modules, must be implemented by derived classes
    bool         ExecScript(const std::string script); // executes a Python script
    bool         SetCurrentDir(const std::string dir); // sets Python current work directory
    bool         AddToSysPath(const std::string dir);  // adds directory to Python sys.patch
    void         SetErrorModule(boost::python::object& module); // sets Python module that contains error report function defined on the Python side
    std::vector<std::string> ErrorReport();            // wraps call to error report function defined on the Python side

private:
    // some helper objects needed to initialize and run the Python interface
#if defined(FREEORION_MACOSX) || defined(FREEORION_WIN32)
    char                  m_home_dir[1024];
    char                  m_program_name[1024];
#endif
    boost::python::dict   m_namespace;

    // used to track if and which Python module contains the "error_report" function ErrorReport should call
    boost::python::object* m_python_module_error;
};

// returns root folder containing all the Python scripts
const std::string GetPythonDir();

// returns folder containing common Python modules used by all Python scripts
const std::string GetPythonCommonDir();


#endif /* defined(__FreeOrion__Python__CommonFramework__) */
