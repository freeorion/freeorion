#include "SourcePythonParser.h"

#include <boost/python/dict.hpp>

#include "../universe/Conditions.h"
#include "../universe/ValueRefs.h"


variable_wrapper variable_wrapper::get_variable_property(std::string_view container) const
{ return variable_wrapper(m_reference_type, ToContainer(container)); }

ValueRef::ContainerType variable_wrapper::ToContainer(std::string_view s) noexcept {
    return (s == "Planet") ? ValueRef::ContainerType::PLANET :
           (s == "System") ? ValueRef::ContainerType::SYSTEM :
           (s == "Fleet") ? ValueRef::ContainerType::FLEET :
           ValueRef::ContainerType::NONE;
}

void RegisterGlobalsSources(boost::python::dict& globals) {
    globals["Source"] = variable_wrapper(ValueRef::ReferenceType::SOURCE_REFERENCE);
    globals["Target"] = variable_wrapper(ValueRef::ReferenceType::EFFECT_TARGET_REFERENCE);
    globals["LocalCandidate"] = variable_wrapper(ValueRef::ReferenceType::CONDITION_LOCAL_CANDIDATE_REFERENCE);
    globals["RootCandidate"] = variable_wrapper(ValueRef::ReferenceType::CONDITION_ROOT_CANDIDATE_REFERENCE);
}

