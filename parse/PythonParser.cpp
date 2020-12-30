#include "PythonParser.h"

#include "../util/Directories.h"
#include "../util/Logger.h"
#include "../util/PythonCommon.h"

#include <stdexcept>

PythonParser::PythonParser(PythonCommon& _python)
    : python(_python)
{
    if (!python.IsPythonRunning()) {
        ErrorLogger() << "Python parse given non-initialized python!";
        throw std::runtime_error("Python isn't initialized");
    }

    try {
        type_int = boost::python::eval("int", boost::python::dict());
        type_float = boost::python::eval("float", boost::python::dict());
        type_bool = boost::python::eval("bool", boost::python::dict());
        type_str = boost::python::eval("str", boost::python::dict());
    } catch (const boost::python::error_already_set& err) {
        python.HandleErrorAlreadySet();
        if (!python.IsPythonRunning()) {
            ErrorLogger() << "Python interpreter is no longer running.  Attempting to restart.";
            if (python.Initialize()) {
                ErrorLogger() << "Python interpreter successfully restarted.";
            } else {
                ErrorLogger() << "Python interpreter failed to restart.  Exiting.";
            }
        }
        throw std::runtime_error("Python isn't initialized");
    }
}

PythonParser::~PythonParser() {
}

bool PythonParser::ParseFileCommon(const boost::filesystem::path& path,
                                   const boost::python::dict& globals,
                                   std::string& filename, std::string& file_contents) const
{
    filename = path.string();

    bool read_success = ReadFile(path, file_contents);
    if (!read_success) {
        ErrorLogger() << "Unable to open data file " << filename;
        return false;
    }

    try {
        boost::python::exec(file_contents.c_str(), globals);
    } catch (const boost::python::error_already_set& err) {
        python.HandleErrorAlreadySet();
        if (!python.IsPythonRunning()) {
            ErrorLogger() << "Python interpreter is no longer running.  Attempting to restart.";
            if (python.Initialize()) {
                ErrorLogger() << "Python interpreter successfully restarted.";
            } else {
                ErrorLogger() << "Python interpreter failed to restart.  Exiting.";
            }
        }
        return false;
    }

    return true;
}

