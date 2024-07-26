#ifndef _SourcePythonParser_h_
#define _SourcePythonParser_h_

#include "ValueRefPythonParser.h"

struct variable_wrapper {
    explicit variable_wrapper(ValueRef::ReferenceType reference_type,
                              ValueRef::ContainerType container = ValueRef::ContainerType::NONE) :
        m_reference_type(reference_type),
        m_container(container)
    {}

    value_ref_wrapper<int> get_int_property(std::string property) const;
    value_ref_wrapper<double> get_double_property(std::string property) const;
    value_ref_wrapper<std::string> get_string_property(std::string property) const;
    variable_wrapper get_variable_property(std::string_view container) const;

    static ValueRef::ContainerType ToContainer(std::string_view s) noexcept;

    const ValueRef::ReferenceType m_reference_type;
    const ValueRef::ContainerType m_container;
};

void RegisterGlobalsSources(boost::python::dict& globals);

#endif // _SourcePythonParser_h_
