#include "PythonFramework.h"

#include "../../util/Directories.h"
#include "../../util/Logger.h"

#include "../PythonWrappers.h"

#include <boost/lexical_cast.hpp>
#include <boost/python.hpp>
#include <boost/python/list.hpp>
#include <boost/python/tuple.hpp>
#include <boost/python/extract.hpp>
#include <boost/date_time/posix_time/time_formatters.hpp>

#ifdef FREEORION_MACOSX
#include <sys/param.h>
#endif

using boost::python::object;
using boost::python::import;
using boost::python::error_already_set;
using boost::python::exec;
using boost::python::dict;
using boost::python::list;
using boost::python::tuple;
using boost::python::make_tuple;
using boost::python::extract;
using boost::python::len;


// Python module for logging functions
BOOST_PYTHON_MODULE(fo_logger) {
    FreeOrionPython::WrapLogger();
}

namespace {
    // Some helper objects needed to initialize and run the
    // Python interface
#ifdef FREEORION_MACOSX
    static char     s_python_home[MAXPATHLEN];
    static char     s_python_program_name[MAXPATHLEN];
#endif
    static dict     s_python_namespace = dict();
}

bool PythonInit() {
    Logger().debugStream() << "Initializing Python interface";

    try {
#ifdef FREEORION_MACOSX
        // There have been recurring issues on OSX to get FO to use the
        // Python framework shipped with the app (instead of falling back
        // on the ones provided by the system). These API calls have been
        // added in an attempt to solve the problems. Not sure if they
        // are really required, but better save than sorry.. ;)
        strcpy(s_python_home, GetPythonHome().string().c_str());
        Py_SetPythonHome(s_python_home);
        Logger().debugStream() << "Python home set to " << Py_GetPythonHome();
        strcpy(s_python_program_name, (GetPythonHome() / "Python").string().c_str());
        Py_SetProgramName(s_python_program_name);
        Logger().debugStream() << "Python program name set to " << Py_GetProgramFullPath();
#endif
        // initializes Python interpreter, allowing Python functions to be called from C++
        Py_Initialize();
        Logger().debugStream() << "Python initialized";
        Logger().debugStream() << "Python version: " << Py_GetVersion();
        Logger().debugStream() << "Python prefix: " << Py_GetPrefix();
        Logger().debugStream() << "Python module search path: " << Py_GetPath();
        Logger().debugStream() << "Initializing C++ interfaces for Python";
        initfo_logger();              // allows the "fo_logger" C++ module to be imported within Python code
    }
    catch (...) {
        Logger().errorStream() << "Unable to initialize Python interpreter";
        return false;
    }

    try {
        // get main namespace, needed to run other interpreted code
        object py_main = import("__main__");
        s_python_namespace = extract<dict>(py_main.attr("__dict__"));
    }
    catch (error_already_set err) {
        Logger().errorStream() << "Unable to set up main namespace in Python";
        PyErr_Print();
        return false;
    }

    // set up logging by redirecting stdout and stderr to exposed logging functions
    std::string script = "import sys\n"
    "import fo_logger\n"
    "class dbgLogger:\n"
    "  def write(self, msg):\n"
    "    fo_logger.log(msg)\n"
    "class errLogger:\n"
    "  def write(self, msg):\n"
    "    fo_logger.error(msg)\n"
    "sys.stdout = dbgLogger()\n"
    "sys.stderr = errLogger()\n"
    "print ('Python stdout and stderr redirected')";
    if (!PythonExecScript(script)) {
        Logger().errorStream() << "Unable to redirect Python stdout and stderr";
        return false;
    }

    Logger().debugStream() << "Python interface successfully initialized!";
    return true;
}

bool PythonExecScript(const std::string script) {
    try { object ignored = exec(script.c_str(), s_python_namespace, s_python_namespace); }
    catch (error_already_set err) {
        PyErr_Print();
        return false;
    }
    return true;
}

bool PythonSetCurrentDir(const std::string dir) {
    std::string script = "import os\n"
    "os.chdir(r'" + dir + "')\n"
    "print 'Python current directory set to', os.getcwd()";
    if (!PythonExecScript(script)) {
        Logger().errorStream() << "Unable to set Python current directory";
        return false;
    }
    return true;
}

bool PythonAddToSysPath(const std::string dir) {
    std::string command = "sys.path.append(r'" + dir + "')";
    if (!PythonExecScript(command)) {
        Logger().errorStream() << "Unable to set universe generator script dir";
        return false;
    }
    return true;
}

void PythonCleanup() {
    Py_Finalize();
    s_python_namespace = dict(); // TODO: is that really necessary?
    Logger().debugStream() << "Cleaned up Python interface";
}
