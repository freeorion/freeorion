#include "PythonCommon.h"

#include <boost/python.hpp>

PythonCommon::PythonCommon() {
}

PythonCommon::~PythonCommon() {
}

bool PythonCommon::IsPythonRunning()
{ return Py_IsInitialized(); }

bool PythonCommon::Initialize()
{ return true; }

