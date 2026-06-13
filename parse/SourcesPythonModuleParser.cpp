#include "SourcesPythonModuleParser.h"

#include <boost/python/docstring_options.hpp>
#include <boost/python/module.hpp>
#include <boost/python/scope.hpp>

#include "../universe/ValueRef.h"

#include "SourcePythonParser.h"


namespace py = boost::python;

BOOST_PYTHON_MODULE(_sources) {
    py::docstring_options doc_options(true, true, false);

    py::scope().attr("Source") = variable_wrapper(ValueRef::ReferenceType::SOURCE_REFERENCE);
    py::scope().attr("Target") = variable_wrapper(ValueRef::ReferenceType::EFFECT_TARGET_REFERENCE);
    py::scope().attr("LocalCandidate") = variable_wrapper(ValueRef::ReferenceType::CONDITION_LOCAL_CANDIDATE_REFERENCE);
    py::scope().attr("RootCandidate") = variable_wrapper(ValueRef::ReferenceType::CONDITION_ROOT_CANDIDATE_REFERENCE);
}