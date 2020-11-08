#include "PythonParser.h"

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
}

PythonParser::~PythonParser() {
}

