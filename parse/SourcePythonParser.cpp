#include "SourcePythonParser.h"

#include <boost/python/dict.hpp>

#include "../universe/Conditions.h"
#include "../universe/ValueRefs.h"

value_ref_wrapper<int> variable_wrapper::get_int_property(std::string property) const {
    return value_ref_wrapper<int>(std::make_shared<ValueRef::Variable<int>>(
        m_reference_type, std::move(property), m_container, ValueRef::ValueToReturn::Initial));
}

value_ref_wrapper<double> variable_wrapper::get_double_property(std::string property) const {
    return value_ref_wrapper<double>(std::make_shared<ValueRef::Variable<double>>(
        m_reference_type, std::move(property), m_container));
}

value_ref_wrapper<std::string> variable_wrapper::get_string_property(std::string property) const {
    return value_ref_wrapper<std::string>(std::make_shared<ValueRef::Variable<std::string>>(
        m_reference_type, std::move(property), m_container));
}

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

