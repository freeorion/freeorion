#include "SourcePythonParser.h"

#include <boost/python/dict.hpp>

#include "../universe/Conditions.h"
#include "../universe/ValueRefs.h"

value_ref_wrapper<int> variable_wrapper::get_int_property(const char *property) const {
    std::vector property_name = std::vector(m_container);
    property_name.emplace_back(property);
    return value_ref_wrapper<int>(std::make_shared<ValueRef::Variable<int>>(m_reference_type, property_name));
}

value_ref_wrapper<double> variable_wrapper::get_double_property(const char *property) const {
    std::vector property_name = std::vector(m_container);
    property_name.emplace_back(property);
    return value_ref_wrapper<double>(std::make_shared<ValueRef::Variable<double>>(m_reference_type, property_name));
}

value_ref_wrapper<std::string> variable_wrapper::get_string_property(const char *property) const {
    std::vector property_name = std::vector(m_container);
    property_name.emplace_back(property);
    return value_ref_wrapper<std::string>(std::make_shared<ValueRef::Variable<std::string>>(m_reference_type, property_name));
}

variable_wrapper variable_wrapper::get_variable_property(const char *property) const {
    return variable_wrapper(m_reference_type, std::vector{std::string(property)});
}

void RegisterGlobalsSources(boost::python::dict& globals) {
    globals["Source"] = variable_wrapper(ValueRef::ReferenceType::SOURCE_REFERENCE);
    globals["Target"] = variable_wrapper(ValueRef::ReferenceType::EFFECT_TARGET_REFERENCE);
    globals["LocalCandidate"] = variable_wrapper(ValueRef::ReferenceType::CONDITION_LOCAL_CANDIDATE_REFERENCE);
    globals["RootCandidate"] = variable_wrapper(ValueRef::ReferenceType::CONDITION_ROOT_CANDIDATE_REFERENCE);
}

