#ifndef _SourcePythonParser_h_
#define _SourcePythonParser_h_

#include "ValueRefPythonParser.h"

struct variable_wrapper {
    variable_wrapper(ValueRef::ReferenceType reference_type)
        : m_reference_type(reference_type)
    {}

    template<typename S>
    variable_wrapper(ValueRef::ReferenceType reference_type, S&& container)
        : m_reference_type(reference_type)
        , m_container(std::move(container))
    {}

    value_ref_wrapper<int> get_int_property(const char *property) const;
    value_ref_wrapper<double> get_double_property(const char *property) const;
    value_ref_wrapper<std::string> get_string_property(const char *property) const;
    variable_wrapper get_variable_property(const char *property) const;

    const ValueRef::ReferenceType m_reference_type;
    const std::vector<std::string> m_container;
};

void RegisterGlobalsSources(boost::python::dict& globals);

#endif // _SourcePythonParser_h_
